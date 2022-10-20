import numpy as np
import warnings
import pycba as cba
from PyBTLS.py_module.misc._helper_functions import from_2darray_to_string

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
    
    #Method to add LE to consider per lane
    #lane_factor argument can be either singular number, list or np array
    #if singular number (float or int), it will apply that lane factor to all lanes
    #if it is a list/np array, it must have the same length as n_lane
    
    def add_load_effect(self, load_effect, lane_factor=1.0, POT = 0.0, poi = None, step = None):
        if (isinstance(lane_factor, int)) or (isinstance(lane_factor, float)):
            lane_factor = np.ones(self.n_lane) * lane_factor
        elif (isinstance(lane_factor, list)) or (isinstance(lane_factor, np.ndarray)):
            if not (len(lane_factor) == self.n_lane):
                raise ValueError("The length of 'lane_factor' argument is not the same as the 'n_lane' property of the bridge")
        else:
            raise ValueError("'lane_factor' argument must be either in list, numpy array, int or float.")
            
        if not poi == None:
            poi = float(poi)
            
        if isinstance(load_effect, int):
            #Using the builtin LE function
            if not (load_effect>0 and load_effect<=7):
                raise ValueError("load_effect argument must be between 1 and 7 (inclusive) for built in IL inside BTLS")
            if not poi == None:
                warnings.warn ("load_effect specified as built in inside BTLS, but poi is supplied. poi argument will be ignored")
            if not step == None:
                warnings.warn ("load_effect specified as built in inside BTLS, but step is supplied. step argument will be ignored")
                
        elif isinstance(load_effect, str):
            if not (load_effect == "M" or load_effect == "V" or load_effect == "R"):
                raise ValueError("load_effect argument must be either 'M', 'V', or 'R'")
            if poi == None:
                raise ValueError("poi argument must be supplied")
        else:
            raise TypeError("load_effect argument must be either between 1 and 7 (inclusive), or 'M', 'V', or 'R'")
            
        if not len(lane_factor) == self.n_lane:
            raise ValueError("lane_factor must have a length equal to the n_lane")
        
        if step == None:
            step = 0.5 #default
        self.load_effect.append({"POT": POT, 
                                 "poi": poi, 
                                 "load_effect": load_effect, 
                                 "lane_factor": lane_factor, 
                                 "step": step
                                })

    
    def _get_il(self, poi, load_effect, step = 0.5):
        ils = cba.InfluenceLines(self.L, np.ones(len(self.L)), self.R)
        ils.create_ils(step)
        return ils.get_il(poi = poi, load_effect = load_effect)
    
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
                il = self._get_il(le["poi"], le["load_effect"], le["step"])
                xy = from_2darray_to_string(np.vstack((il[0], il[1])).T)
                #Add to the IL text
                #ilt += LE ID, Number of pt in this LE; xy coordinates of IL
                ilt += str(custom_le_id) + "," + str(len(il[0])) + "\n" + xy
        
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