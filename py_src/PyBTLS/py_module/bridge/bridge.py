import numpy as np
import warnings
import pycba as cba
from PyBTLS.py_module.utils._helper_functions import from_2darray_to_string, maxdistance

"""

"""

class Bridge():
    def __init__(self, L, R, n_lane):
        self.L = L
        self.R = R
        self._n_lane = n_lane
        self.load_effect = []
        self._bridge_txt = None
        self._il_txt = None
    
    @property
    def n_lane(self):
        return self._n_lane
    @n_lane.setter
    def n_lane(self):
        raise ValueError("Cannot change n_lane after initialisation. Please create another instance of BTLSBridge to change the 'n_lane' property")
    """
    Method to add LE to consider per lane
    lane_factor argument can be either singular number, list or np array
    if singular number (float or int), it will apply that lane factor to all lanes
    if it is a list/np array, it must have the same length as n_lane
    Arguments:
        load_effect: 
            load effect to be considered. If integer between 1 - 7 inclusive, uses BTLS built-in IL
            Otherwise can be "M", "V", or "R" for moment, shear and reaction respectively. If using
            "M", "V" or "R", must supply `poi` argument
        poi:
            The point of interest for the IL. Will be ignored if using built-in BTLS IL, and a warning
            will be thrown.
        max_error: (Default to 0.001)
            Maximum relative error between the IL to be used by BTLS in the true IL as computed by 
            pycba. The default value is 0.001, corresponding to a maximum error of 0.1%. Smaller number
            slows down the simulation.
        min_step: (Default to 0.1)
            Minimum allowable **equally sized** step size by the IL. That is, it will be converted to max
            number of points that the IL can have. If the relative error is not achieved even until this
            point is reached, it will switch to the equal spaced IL and warn the user. Larger number
            speeds up simulation at a cost of IL accuracy. Default to 0.1 meter
    """
    def add_load_effect(self, load_effect, lane_factor=1.0, POT = 0.0, poi = None, max_error = None, min_step = None):
        if (isinstance(lane_factor, int)) or (isinstance(lane_factor, float)):
            lane_factor = np.ones(self.n_lane) * lane_factor
        elif (isinstance(lane_factor, list)) or (isinstance(lane_factor, np.ndarray)):
            if not (len(lane_factor) == self.n_lane):
                raise ValueError("The length of 'lane_factor' argument is not the same as the 'n_lane' property of the bridge")
        else:
            raise ValueError("'lane_factor' argument must be either in list, numpy array, int or float.")
            
        if not poi is None:
            poi = float(poi)
            
        if isinstance(load_effect, int):
            #Using the builtin LE function
            if not (load_effect>0 and load_effect<=7):
                raise ValueError("load_effect argument must be between 1 and 7 (inclusive) for built in IL inside BTLS")
            if not poi is None:
                warnings.warn ("load_effect specified as built in inside BTLS, but poi is supplied. poi argument will be ignored")
            if not max_error is None:
                warnings.warn ("load_effect specified as built in inside BTLS, but max_error is supplied. max_error argument will be ignored")
            if not min_step is None:
                warnings.warn ("load_effect specified as built in inside BTLS, but max_npts is supplied. max_npts argument will be ignored")


        elif isinstance(load_effect, str):
            if not (load_effect == "M" or load_effect == "V" or load_effect == "R"):
                raise ValueError("load_effect argument must be either 'M', 'V', or 'R'")
            if poi == None:
                raise ValueError("poi argument must be supplied")
        else:
            raise TypeError("load_effect argument must be either between 1 and 7 (inclusive), or 'M', 'V', or 'R'")
            
        if not len(lane_factor) == self.n_lane:
            raise ValueError("lane_factor must have a length equal to the n_lane")
        
        if max_error == None:
            max_error = 0.001 #default
        if min_step == None:
            min_step = 0.1 #m, default
        self.load_effect.append({"POT": POT, 
                                 "poi": poi, 
                                 "load_effect": load_effect, 
                                 "lane_factor": lane_factor, 
                                 "max_error": max_error,
                                 "min_step": min_step,
                                })

    
    def _get_il(self, poi, load_effect, max_error = 0.001, min_step = 0.1):

        #IL to be compressed
        ils = cba.InfluenceLines(self.L, np.ones(len(self.L)), self.R)
        ils.create_ils(0.01) #Initial step sizes
        ilx, ily = ils.get_il(poi = poi, load_effect = load_effect)
        ilx, ily = maxdistance(ilx, ily, max_error) #Compressed IL

        #Calculate max number of points
        max_npts = np.sum(self.L) / min_step
        if len(ilx) > max_npts:
            ils = cba.InfluenceLines(self.L, np.ones(len(self.L)), self.R)
            ils.create_ils(min_step) #Initial step sizes
            ilx, ily = ils.get_il(poi = poi, load_effect = load_effect) #Compressed IL
            warnings.warn("Cannot achieve required max_error given min_step. IL reverting to the min_step.")

        return ilx, ily
    
    def _create_txt(self):
        brt = ""
        ilt = ""
        n_lane = self.n_lane
        n_le = len(self.load_effect)
        l = np.sum(self.L) #Total length
        custom_le_id = 0
        
        brt += "1," + str(l) + "," + str(n_lane) + "," + str(n_le) + "\n"
        for j, le in enumerate(self.load_effect):
            brt += str(j+1) + ",1," + str(le["POT"]) + "\n"
            if isinstance(le["load_effect"], int): #Use built in IL
                brt += "1," + str(le["load_effect"])
                for k, lf in enumerate(le["lane_factor"]):
                     brt += "," + str(lf)
                brt += "\n"

            else: #Define new IL
                #For the bridge file
                custom_le_id += 1
                brt += "2," + str(custom_le_id)
                for k, lf in enumerate(le["lane_factor"]):
                     brt += "," + str(lf)
                brt += "\n"
                
                #For the IL file, create new IL 
                il = self._get_il(le["poi"], le["load_effect"], le["max_error"], le["min_step"])
                xy = from_2darray_to_string(np.vstack((il[0], il[1])).T)
                #Add to the IL text
                #ilt += LE ID, Number of pt in this LE; xy coordinates of IL
                ilt += str(custom_le_id) + "," + str(len(il[0])) + "\n" + xy + "\n"
        
        #At the end, append the number of IL in the IL file
        ilt = str(custom_le_id) + "\n" + ilt
        
        #If there was no custom IL, override and create dummy IL. Necessary for BTLS to run (?!?!)
        if custom_le_id == 0:
            ilt = "1\n"
            ilt+= "1,2\n"
            ilt+= "0,0\n"
            ilt+= str(l)+",0\n"
        
        self._bridge_txt = brt
        self._il_txt = ilt
        
    def _get_bridge_txt(self):
        return self._bridge_txt
    
    def _get_il_txt(self):
        return self._il_txt

    def __str__(self):
        qualname = type(self).__qualname__
        return f"{self.n_lane} lane(s) {self.L} m {qualname} object at {hex(id(self))}"

    def __repr__(self):
        return self.__str__()