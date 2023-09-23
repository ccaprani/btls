import numpy as np
import warnings
import pycba as cba
from PyBTLS.py.utils_interface._helper_functions import (
    from_2darray_to_string,
    maxdistance,
)

class Bridge:
    """
    Python side Bridge Class.
    
    """
    def __init__(self, L, R, n_lane):
        """
        This Bridge class contains the bridge properties, e.g., length, restraint, and number of lanes.
        This class will compute influence lines automatically, given a set of load effects that the user wants to analyze.
        It will also write all the necessary bridge files for BTLS to run.

        Parameters
        ----------
        L : list or numpy array
            Length of each span of the bridge. 
        R: list or numpy array
            Restraint condition of each span of the bridge. 
            See PyCBA documentation for more details on the restraint argument.
        n_lane : int
            Number of lanes of the bridge.
        """
        self.L = L
        self.R = R
        self._n_lane = n_lane
        self.load_effect = []
        self._bridge_txt = None
        self._il_txt = None

    @property
    def n_lane(self):
        """
        Get the number of lanes. This is a read only property. To change the number of lanes, please create a new instance of this object.
        """
        return self._n_lane

    @n_lane.setter
    def n_lane(self):
        raise ValueError(
            "Cannot change n_lane after initialisation. Please create another instance of BTLSBridge to change the 'n_lane' property"
        )

    def add_load_effect(
        self,
        load_effect,
        lane_factor=1.0,
        POT=0.0,
        poi=None,
        max_error=None,
        min_step=None,
    ):
        """
        Method to add load effects to be analyzed per lane.

        Parameters
        ----------
        load_effect : int or str
            load effect to be considered. If integer between 1 - 7 inclusive, uses BTLS built-in influence lines.
            Otherwise can be "M", "V", or "R" for moment, shear and reaction respectively. 
            If using "M", "V" or "R", must supply `poi` argument.
        lane_factor : float, list, or numpy array, optional
            Lane factor to be applied to the load effect. The default is 1.0.
            Useful when analyzing multiple lanes, where some lanes can be set to have smaller load effect than other.
            Can be either singular number, list, or numpy array.
            If singular number (float or int), it will apply that lane factor to all lanes.
            If it is a list/np array, it must have the same length as n_lane.
        POT : float, optional
            Threshold for Peak Over Threshold analysis.
            Load effects observations below POT is discarded by BTLS.
            The default is 0.0 (No discarded observations).
        poi : float, optional
            The point of interest for the influence line, if supplying "M", "V", or "R".
            e.g., if you want to analyze shear at quarter of the bridge **length**, then set poi to 0.25. 
            This will be ignored if using built-in BTLS IL, and a warning will be thrown.
        max_error : float, optional
            Maximum relative error between the influence lines to be used by BTLS compared to the true influence lines computed by PyCBA. 
            The default value is 0.001, corresponding to a maximum error of 0.1%. 
            Smaller number will slow down the simulation.
        min_step: (Default to 0.1)
            Minimum allowable **equally sized** step size by the influence lines. 
            That is, it will be converted to max number of points that the influence lines can have. 
            If the relative error is not achieved when this point is reached, it will switch to the equal spaced influence line and warn the user. 
            Larger number speeds up simulation at a cost of influence lines accuracy. 
            Default to 0.1 metre.
        """
        if (isinstance(lane_factor, int)) or (isinstance(lane_factor, float)):
            lane_factor = np.ones(self.n_lane) * lane_factor
        elif (isinstance(lane_factor, list)) or (isinstance(lane_factor, np.ndarray)):
            if not (len(lane_factor) == self.n_lane):
                raise ValueError(
                    "The length of 'lane_factor' argument is not the same as the 'n_lane' property of the bridge"
                )
        else:
            raise ValueError(
                "'lane_factor' argument must be either in list, numpy array, int or float."
            )

        if not poi is None:
            poi = float(poi)

        if isinstance(load_effect, int):
            # Using the builtin LE function
            if not (load_effect > 0 and load_effect <= 7):
                raise ValueError(
                    "load_effect argument must be between 1 and 7 (inclusive) for built in IL inside BTLS"
                )
            if not poi is None:
                warnings.warn(
                    "load_effect specified as built in inside BTLS, but poi is supplied. poi argument will be ignored"
                )
            if not max_error is None:
                warnings.warn(
                    "load_effect specified as built in inside BTLS, but max_error is supplied. max_error argument will be ignored"
                )
            if not min_step is None:
                warnings.warn(
                    "load_effect specified as built in inside BTLS, but max_npts is supplied. max_npts argument will be ignored"
                )

        elif isinstance(load_effect, str):
            if not (load_effect == "M" or load_effect == "V" or load_effect == "R"):
                raise ValueError("load_effect argument must be either 'M', 'V', or 'R'")
            if poi == None:
                raise ValueError("poi argument must be supplied")
        else:
            raise TypeError(
                "load_effect argument must be either between 1 and 7 (inclusive), or 'M', 'V', or 'R'"
            )

        if not len(lane_factor) == self.n_lane:
            raise ValueError("lane_factor must have a length equal to the n_lane")

        if max_error == None:
            max_error = 0.001  # default
        if min_step == None:
            min_step = 0.1  # m, default
        self.load_effect.append(
            {
                "POT": POT,
                "poi": poi,
                "load_effect": load_effect,
                "lane_factor": lane_factor,
                "max_error": max_error,
                "min_step": min_step,
            }
        )

    def _get_il(self, poi, load_effect, max_error=0.001, min_step=0.1):
        """
        Private method to get influence lines.
        IL computed using PyCBA, then compressed using the maxdistance algorithm.
        """
        # IL to be compressed
        ils = cba.InfluenceLines(self.L, np.ones(len(self.L)), self.R)
        ils.create_ils(0.01)  # Initial step sizes
        ilx, ily = ils.get_il(poi=poi, load_effect=load_effect)
        ilx, ily = maxdistance(ilx, ily, max_error)  # Compressed IL

        # Calculate max number of points
        max_npts = np.sum(self.L) / min_step
        if len(ilx) > max_npts:
            ils = cba.InfluenceLines(self.L, np.ones(len(self.L)), self.R)
            ils.create_ils(min_step)  # Initial step sizes
            ilx, ily = ils.get_il(poi=poi, load_effect=load_effect)  # Compressed IL
            warnings.warn(
                "Cannot achieve required max_error given min_step. IL reverting to the min_step."
            )

        return ilx, ily

    def _create_il_txt(self):
        """
        Private method to create the influence line text string for BTLS to run.
        This will be automatically called when running BTLS.
        """
        ilt = ""
        l = np.sum(self.L)  # Total length
        custom_le_id = 0

        for j, le in enumerate(self.load_effect):
            if not isinstance(le["load_effect"], int):  # Define new IL
                custom_le_id += 1
                # For the IL file, create new IL
                il = self._get_il(
                    le["poi"], le["load_effect"], le["max_error"], le["min_step"]
                )
                xy = from_2darray_to_string(np.vstack((il[0], il[1])).T)
                # Add to the IL text
                # ilt += LE ID, Number of pt in this LE; xy coordinates of IL
                ilt += str(custom_le_id) + "," + str(len(il[0])) + "\n" + xy + "\n"

        # At the end, append the number of IL in the IL file
        ilt = str(custom_le_id) + "\n" + ilt

        # If there was no custom IL, override and create dummy IL. Necessary for BTLS to run (?!?!)
        if custom_le_id == 0:
            ilt = "1\n"
            ilt += "1,2\n"
            ilt += "0,0\n"
            ilt += str(l) + ",0\n"

        self._il_txt = ilt
        return ilt

    def _create_bridge_txt(self):
        """
        Private method to create the bridge text string for BTLS to run.
        This will be automatically called when running BTLS.
        """
        brt = ""
        n_lane = self.n_lane
        n_le = len(self.load_effect)
        l = np.sum(self.L)  # Total length
        custom_le_id = 0

        brt += "1," + str(l) + "," + str(n_lane) + "," + str(n_le) + "\n"
        for j, le in enumerate(self.load_effect):
            brt += str(j + 1) + ",1," + str(le["POT"]) + "\n"
            if isinstance(le["load_effect"], int):  # Use built in IL
                brt += "1," + str(le["load_effect"])
                for k, lf in enumerate(le["lane_factor"]):
                    brt += "," + str(lf)
                brt += "\n"

            else:  # Define new IL
                # For the bridge file
                custom_le_id += 1
                brt += "2," + str(custom_le_id)
                for k, lf in enumerate(le["lane_factor"]):
                    brt += "," + str(lf)
                brt += "\n"

        self._bridge_txt = brt
        return brt

    def _get_bridge_txt(self):
        """
        Private method to get the bridge text string.
        """
        return self._bridge_txt

    def _get_il_txt(self):
        """
        Private method to get the influence lines text string.
        """
        return self._il_txt

    def write_bridge_as_txt(self, path="bridge.txt"):
        """
        Method to write the bridge text file for BTLS to run.
        """
        with open(path, "w") as f:
            f.write(self._create_bridge_txt())

    def write_il_as_txt(self, path="IL.txt"):
        """
        Method to write the influence line text file for BTLS to run.
        """
        with open(path, "w") as f:
            f.write(self._create_il_txt())

    def write_as_txt(self, il_path="IL.txt", bridge_path="bridge.txt"):
        """
        Method to write both the bridge and influence line text file for BTLS to run.
        """
        self.write_bridge_as_txt(bridge_path)
        self.write_il_as_txt(il_path)

    def __str__(self):
        qualname = type(self).__qualname__
        return f"{self.n_lane} lane(s) {self.L} m {qualname} object at {hex(id(self))}"

    def __repr__(self):
        return self.__str__()
