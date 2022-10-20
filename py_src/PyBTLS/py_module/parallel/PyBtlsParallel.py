#import PyBTLS
import PyBtlsOutput
import subprocess
import numpy as np
import pycba as cba
import warnings
import os
import shutil
from glob import glob
from _helper_functions import from_2darray_to_string
import pickle


r"""

"""

class BTLSBridge():
    def __init__(self, L, R, n_lane):
        self.L = L
        self.R = R
        self._n_lane = n_lane
        self.load_effect = []
        self._bridge_txt = None
        self._il_txt = None
        
    def get_il(self, poi, load_effect, step = 0.5):
        ils = cba.InfluenceLines(self.L, np.ones(len(self.L)), self.R)
        ils.create_ils(step)
        return ils.get_il(poi = poi, load_effect = load_effect)
    
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
    
    def create_txt(self):
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
                il = self.get_il(le["poi"], le["load_effect"], le["step"])
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
        
    def get_bridge_txt(self):
        return self._bridge_txt
    
    def get_il_txt(self):
        return self._il_txt

r"""

"""
        
class _BtlsWrapper_DefaultOptions():
    def __init__(self, num_days = 1):
        # Program Mode (1 - Gen & Sim, 2 - Gen, 3 - Read & Sim)
        self._program_mode = 1
        # No. of days of traffic simulation:
        self.num_days = num_days
        # Default truck track width (cm)
        self.track_width = 190.0 #cm
        # Standard deviation of eccentricity in lane (cm) (about 20 cm usually)
        self.eccentricity_std_dev = 20 #cm
        # Vehicle Generation Model to be used
        # (0 - Grave, 1 - Constant, 2 - Garage)
        self._vehicle_generation_model = 0
        # Headway model to be used:
        # (0 - Auxerre NHM, 1 - Constant, 5 - Congestion (w/ or w/out cars), 6 - free-flow, cars included)
        self._headway_model = 5
        # Traffic Classification System to be used:
        # (0 - No. of Axles, 1 - Axle Pattern)
        self.traffic_classification = 1 
        # Nominal congested spacing, front to back (m):
        self.nom_congested_spacing = 26.1 #m
        # Congested speed (km/h):
        self.congested_speed = 49.7 #km/h
        # Congested gaps coefficient of variation:
        self.congested_gaps_cov = 0.05
        
        # Impose constant speed on all vehicles (1 or 0):
        self.impose_constant_speed = 0
        # Use average speed of vehicles in file if constant speed imposed (1 or 0)
        self.use_avg_speed_if_impose_constant_speed = 1
        # Constant speed of vehicles if not average used (km/h):
        self.constant_speed_if_not_use_avg_speed = 88 #km/h
        
        # Simulation time step (s)
        self.time_step = 0.1 #s
        # Minimum GVW for inclusion in calculations (t/10):
        self.min_GVW = 0.0
        
        # OUTPUT PARAMETERS
        # Write full time history - slow & large file (1 or 0):
        self.write_full_time_history = 0
        # Write each loading event value (1 or 0):
        self.write_each_loading_event = 0
        # Write each event buffer size:
        self.write_each_loading_event_buffer_size = 10000
        # Write a fatigue event file (1 or 0)
        self.write_fatigue_event = 0
        # Conduct Rainflow algorithm (1 or 0)
        self.write_rainflow_algorithm = 0
        # Number of decimal left for Rainflow algorithm
        self.num_decimal_rainflow_algorithm = 3
        # Cutoff value for Fatigue
        self.fatigue_cutoff_value = 5.0
        
        # Write vehicle file (1 or 0)
        # WARNING: a large file may result in long-run simulations
        self.write_vehicle_file = 0
        # Traffic output file format (CASTOR - 1, BeDIT - 2, DITIS - 3, MON - 4):
        self._write_vehicle_file_format = 2
        # Vehicle file name
        #./Output/Vehicle File.txt
        # Vehicle file buffer size
        self.write_vehicle_file_buffer_size = 100000
        # Write vehicle file flow statistics (1 or 0)
        self.write_vehicle_flow_statistics = 1
        
        # Analyse for Block Max (overrides remaining params) (1 or 0)
        self.analyse_block_max = 0
        # Block size for maxima (days):
        self.size_block_max_in_days = 1
        # Block size for maxima (seconds):
        self.size_block_max_in_seconds = 0
        # Write block max separated vehicle files (1 or 0):
        self.write_block_max_separated_vehicle_files = 0
        # Write block max summary files (1 or 0):
        self.write_block_max_summary_files = 1
        # Do and write block max mixed vehicle analysis (1 or 0):
        self.write_block_max_mixed_analysis = 0
        # Write block max buffer size:
        self.write_block_max_buffer_size = 2
        
        # Analyse for POT (overrides remaining params) (1 or 0)
        self.analyse_pot = 0
        # Write POT vehicle files (1 or 0):
        self.write_pot_vehicle_files = 0
        # Write POT summary files (1 or 0):
        self.write_pot_summary_files = 1
        # Write POT counter files (1 or 0):
        self.write_pot_counter_files = 1
        # POT counter size (days):
        self.pot_counter_size_in_days = 1
        # POT counter (seconds):
        self.pot_counter_size_in_seconds = 0
        # Write POT buffer size:
        self.write_pot_buffer_size = 10000
        
        # Analyse for Statistics (overrides remaining params) (1 or 0)
        self.analyse_statistics = 1
        # Write cumulative statistics file (1 or 0)
        self.write_cumulative_statistics_file = 1
        # Write statistics at intervals files (1 or 0)
        self.write_interval_statistics_files = 0
        # Interval size for statistics output (seconds)
        self.interval_statistics_size = 3600
        # Write interval statistics buffer size:
        self.interval_statistics_buffer_size = 10000
        
        
        
    @property
    def program_mode(self):
        match self._program_mode:
            case 1:
                return "Gen & Sim"
            case 2:
                return "Gen"
            case 3:
                return "Read & Sim"

    @program_mode.setter
    def program_mode(self, mode):
        if mode in [1,2,3,"Gen & Sim", "Gen", "Read & Sim"]:
            match mode:
                case "Gen & Sim":
                    self._program_mode = 1
                case "Gen":
                    self._program_mode = 2
                case "Read & Sim":
                    self._program_mode = 3
                case _:
                    self._program_mode = mode
        else:
            raise ValueError('program_mode can be either 1,2,3, or "Gen & Sim", "Gen", "Read & Sim" respectively')
    
    @property
    def vehicle_generation_model(self):
        match self._vehicle_generation_model:
            case 0:
                return "Grave"
            case 1:
                return "Constant"
            case 2:
                return "Garage"
    
    @vehicle_generation_model.setter
    def vehicle_generation_model(self, model):
        if model in [0,1,2,"Grave", "Constant", "Garage"]:
            match model:
                case "Grave":
                    self._vehicle_generation_model = 0
                case "Constant":
                    self._vehicle_generation_model = 1
                case "Garage":
                    self._vehicle_generation_model = 2
                case _:
                    self._vehicle_generation_model = model
        else:
            raise ValueError('vehicle_generation_model can be either 0,1,2, or "Grave", "Constant", "Garage" respectively')
    
    
    @property
    def headway_model(self):
        match self._headway_model:
            case 0:
                return "Auxerre NHM"
            case 1:
                return "Constant"
            case 5:
                return "Congestion"
            case 6:
                return "Free Flow"
            
    @headway_model.setter
    def headway_model(self, model):
        if model in [0,1,5,6,"Auxerre NHM", "Constant", "Congestion", "Free Flow"]:
            match model:
                case "Auxerre NHM":
                    self._headway_model = 0
                case "Constant":
                    self._headway_model = 1
                case "Congestion":
                    self._headway_model = 5
                case "Free Flow":
                    self._headway_model = 6
                case _:
                    self._headway_model = model
        else:
            raise ValueError('headway_model can be either 0,1,5,6, or "Auxerre NHM", "Constant", "Congestion", "Free Flow" respectively')
            
    @property
    def write_vehicle_file_format(self):
        match self._write_vehicle_file_format:
            case 1:
                return "CASTOR"
            case 2:
                return "BeDIT"
            case 3:
                return "DITIS"
            case 4:
                return "MON"
            
    @write_vehicle_file_format.setter
    def write_vehicle_file_format(self, file_format):
        if file_format in [1,2,3,4,"CASTOR","BeDIT","DITIS","MON"]:
            match file_format:
                case "CASTOR":
                    self._write_vehicle_file_format = 1
                case "BeDIT":
                    self._write_vehicle_file_format = 2
                case "DITIS":
                    self._write_vehicle_file_format = 3
                case "MON":
                    self._write_vehicle_file_format = 4
                case _:
                    self._write_vehicle_file_format = file_format
        else:
            raise ValueError('output_vehicle_file_format can be either 1,2,3,4, or "CASTOR","BeDIT","DITIS","MON" respectively')


"""
Wrapper class for BTLS. Different to the BTLS object from PyBTLS. This class create a file that will then run the PyBTLS.BTLS object.

Input:
    
"""

class BtlsWrapper():
    def __init__(self, bridge, working_dir = None, num_days = 1):
        self._working_dir = working_dir
        self.options = _BtlsWrapper_DefaultOptions(num_days)
        
        self.bridge = bridge
        
        #Input files
        self.traffic = None
        self.garage = None
        self.kernels = None
        self.bridge_flow = None
        self.constant_vehicle = None
        
        self._traffic_input = None
        self._traffic_input_file = None
        self._traffic_input_file_format = None  # Traffic input file format (CASTOR - 1, BeDIT - 2, DITIS - 3, MON - 4):
        
        self._outputs = PyBtlsOutput._BtlsWrapper_DefaultOutputs()
        
    @property
    def outputs(self):
        return self._outputs
        
    @property
    def working_dir(self):
        return self._working_dir
    
    @working_dir.setter
    def working_dir(self, working_dir):
        if self.working_dir == None:
            self._working_dir = working_dir
        else:
            fpath = self.working_dir
            if os.path.exists(fpath):
                try:
                    os.rmdir(fpath)
                except:
                        raise OSError("Current working directory already exist. Either delete it manually, or use 'force_set_working_dir' method instead")
            self._working_dir = working_dir
        
    def force_set_working_dir(self, working_dir):
        if self.working_dir == None:
            self._working_dir = working_dir
        else:
            fpath = self.working_dir
            if os.path.exists(fpath):
                try:
                    os.rmdir(fpath)
                except:
                    warnings.warn("Working directory already existed, and so was deleted")
                    try:
                        shutil.rmtree(fpath)
                    except:
                        1
            self._working_dir = working_dir
                
    
    @property
    def traffic_input_file_format(self):
        match self._traffic_input_file_format:
            case 1:
                return "CASTOR"
            case 2:
                return "BeDIT"
            case 3:
                return "DITIS"
            case 4:
                return "MON"
            
    @traffic_input_file_format.setter
    def traffic_input_file_format(self, file_format):
        if file_format in [1,2,3,4,"CASTOR","BeDIT","DITIS","MON"]:
            match file_format:
                case "CASTOR":
                    self._traffic_input_file_format = 1
                case "BeDIT":
                    self._traffic_input_file_format = 2
                case "DITIS":
                    self._traffic_input_file_format = 3
                case "MON":
                    self._traffic_input_file_format = 4
                case _:
                    self._traffic_input_file_format = file_format
        else:
            raise ValueError('traffic_input_file_format can be either 1,2,3,4, or "CASTOR","BeDIT","DITIS","MON" respectively')
            
    def _create_working_dir(self):
        os.makedirs("./" + self.working_dir, exist_ok=True)
        os.makedirs("./" + self.working_dir + "/Input/", exist_ok = True)
        os.makedirs("./" + self.working_dir + "/Output/", exist_ok = True)
        
    def _delete_working_dir(self):
        try:
            shutil.rmtree("./" + self.working_dir)
            return True
        except :
            return False
        
    def set_traffic(self, traffic):
        1
        
    def _check_input_completeness(self):
        if self.options.vehicle_generation_model == "Grave":
            if (self.traffic == None):
                warnings.warn("'BtlsTraffic' required for 'Grave' vehicle_generation_model")
                return False
        elif self.options.vehicle_generation_model == "Constant":
            if (self.constant_vehicle == None):
                warnings.warn("'BtlsConstantVehicle' required for 'Constant' vehicle_generation_model")
                return False
        elif self.options.vehicle_generation_model == "Garage":
            if (self.garage == None) or (self.kernels == None):
                warnings.warn("'BtlsGarage' and 'BtlsKernels' required for 'Garage' vehicle_generation_model")
                return False
                
        if self.bridge_flow == None:
            warnings.warn("'BtlsBridgeFlow' required")
            return False
        
        if self.options.program_mode == "Read & Sim":
            if self._traffic_input == None:
                warnings.warn("pregenerated traffic input file required for 'Read & Sim' program_mode. Set using traffic_input property")
                return False
            if self.traffic_input_file_format == None:
                warnings.warn("Traffic Input File Format not set. Assume BeDIT file format. Set using 'traffic_input_file_format' property")
            
        return True
    
    def _create_traffic_files(self, traffic):
        1
        
    def _create_BTLSin_txt(self):
        if self.working_dir == None:
            raise ValueError("ERROR: working directory not set. Please set using working_dir property")
    
        BTLSin_txt = str(self.options._program_mode) + "\n"
        BTLSin_txt += str(self.options.num_days) + "\n"
        BTLSin_txt += "./Traffic/\n"
        BTLSin_txt += str(self.options.track_width) + "\n"
        BTLSin_txt += str(self.options.eccentricity_std_dev) + "\n"
        BTLSin_txt += str(self.options._vehicle_generation_model) + "\n"
        BTLSin_txt += str(self.options._headway_model) + "\n"
        BTLSin_txt += str(self.options.traffic_classification) + "\n"
        BTLSin_txt += "./Input/LaneFlowData.csv\n"
        BTLSin_txt += str(self.options.nom_congested_spacing) + "\n"
        BTLSin_txt += str(self.options.congested_speed) + "\n"
        BTLSin_txt += str(self.options.congested_gaps_cov) + "\n"
        BTLSin_txt += "No garage file\n" if self.garage == None else "./Input/garage.txt\n"
        BTLSin_txt += "No kernels file\n" if self.kernels == None else "./Input/kernels.csv\n"
        BTLSin_txt += "No constant_vehicle file\n" if self.constant_vehicle == None else "./Input/constant_vehicle.csv\n"
        BTLSin_txt += "No traffic input file\n" if self._traffic_input == None else "./Input/traffic_input_file.txt\n"
        BTLSin_txt += "2\n" if self._traffic_input_file_format == None else str(self._traffic_input_file_format) + "\n"
        BTLSin_txt += str(self.options.impose_constant_speed) + "\n"
        BTLSin_txt += str(self.options.use_avg_speed_if_impose_constant_speed) + "\n"
        BTLSin_txt += str(self.options.constant_speed_if_not_use_avg_speed) + "\n"
        BTLSin_txt += "./Input/bridge.txt\n"
        BTLSin_txt += "./Input/IL.txt\n"
        BTLSin_txt += "./Input/IS.txt\n"
        BTLSin_txt += str(self.options.time_step) + "\n"
        BTLSin_txt += str(self.options.min_GVW) + "\n"
        
        BTLSin_txt += str(self.options.write_full_time_history) + "\n"
        BTLSin_txt += str(self.options.write_each_loading_event) + "\n"
        BTLSin_txt += str(self.options.write_each_loading_event_buffer_size) + "\n"
        BTLSin_txt += str(self.options.write_fatigue_event) + "\n"
        BTLSin_txt += str(self.options.write_rainflow_algorithm) + "\n"
        BTLSin_txt += str(self.options.num_decimal_rainflow_algorithm) + "\n"
        BTLSin_txt += str(self.options.fatigue_cutoff_value) + "\n"
        
        
        BTLSin_txt += str(self.options.write_vehicle_file) + "\n"
        BTLSin_txt += str(self.options._write_vehicle_file_format) + "\n"
        BTLSin_txt += "./Output/output_vehicle_file.txt\n"
        BTLSin_txt += str(self.options.write_vehicle_file_buffer_size) + "\n"
        BTLSin_txt += str(self.options.write_vehicle_flow_statistics) + "\n"
        
        BTLSin_txt += str(self.options.analyse_block_max) + "\n"
        BTLSin_txt += str(self.options.size_block_max_in_days) + "\n"
        BTLSin_txt += str(self.options.size_block_max_in_seconds) + "\n"
        BTLSin_txt += str(self.options.write_block_max_separated_vehicle_files) + "\n"
        BTLSin_txt += str(self.options.write_block_max_summary_files) + "\n"
        BTLSin_txt += str(self.options.write_block_max_mixed_analysis) + "\n"
        BTLSin_txt += str(self.options.write_block_max_buffer_size) + "\n"
        
        BTLSin_txt += str(self.options.analyse_pot) + "\n"
        BTLSin_txt += str(self.options.write_pot_vehicle_files) + "\n"
        BTLSin_txt += str(self.options.write_pot_summary_files) + "\n"
        BTLSin_txt += str(self.options.write_pot_counter_files) + "\n"
        BTLSin_txt += str(self.options.pot_counter_size_in_days) + "\n"
        BTLSin_txt += str(self.options.pot_counter_size_in_seconds) + "\n"
        BTLSin_txt += str(self.options.write_pot_buffer_size) + "\n"
        
        BTLSin_txt += str(self.options.analyse_statistics) + "\n"
        BTLSin_txt += str(self.options.write_cumulative_statistics_file) + "\n"
        BTLSin_txt += str(self.options.write_interval_statistics_files) + "\n"
        BTLSin_txt += str(self.options.interval_statistics_size) + "\n"
        BTLSin_txt += str(self.options.interval_statistics_buffer_size) + "\n"
        
        return BTLSin_txt
        
    def create_BTLSin_file(self):
        with open(self.working_dir + "/BTLSin.txt" , 'w') as f:
            f.write(self._create_BTLSin_txt())
        
    def create_BTLS_bridge_files(self):
        self.bridge.create_txt()
        with open(self.working_dir + "/Input/bridge.txt", 'w') as f:
            f.write(self.bridge.get_bridge_txt())
        with open(self.working_dir + "/Input/IL.txt", 'w') as f:
            f.write(self.bridge.get_il_txt())
    
    def create_BTLS_input_files(self):
        if not self.constant_vehicle == None:
            self.constant_vehicle.write_as_csv(self.working_dir + "/Input/constant_vehicle.csv")
        if not self.bridge_flow == None:
            self.bridge_flow.write_as_csv(self.working_dir + "/Input/LaneFlowData.csv")
        if not self.traffic == None:
            self.traffic.write_as_csv(self.working_dir + "/Traffic")
        if not self.garage == None:
            self.garage.write_as_csv(path = self.working_dir + "/Input", fname = "garage.txt")
        if not self.kernels == None:
            self.kernels.write_as_csv(path = self.working_dir + "/Input", fname = "kernels.txt")
        
    def create_py_file(self):
        content = "import PyBTLS;\n" +\
        'import os;\n' +\
        'os.chdir("' + self.working_dir + '");\n' +\
        "btls = PyBTLS.BTLS();\n" +\
        'btls.run("BTLSin.txt");\n'
        with open(self.working_dir + "/BTLS_run.py", 'w') as f:
            f.write(content)
    
    def run_py_file(self):
        process = subprocess.Popen(["python", self.working_dir + "/BTLS_run.py"])
        process.wait()
    
    def read_outputs(self):
        if self.options.write_interval_statistics_files:
            self.outputs.interval_statistics = self._read_interval_statistics_file()
        if (self.options.write_vehicle_file == 1) or (self.options.program_mode == "Gen"):
            self.outputs.vehicles = self._read_generated_vehicles_file()
        if (self.options.analyse_block_max == 1) and (self.options.write_block_max_summary_files == 1):
            self.outputs.block_maxima_summary = self._read_block_maxima_summary_file()
            
        
    def _read_interval_statistics_file(self):
        #There should be N number of files, where N is the number of laod effects
        N_LE = len(self.bridge.load_effect)
        out = []
        for i in range(0, N_LE):
            desc_name = "Load effect " + str(i)
            fpath = glob(self.working_dir+"/SS_S_*_Eff_" + str(i+1) + ".txt") #There should only be one file with this name
            if len(fpath)>1:
                warnings.warn("WARNING: Multiple Interval Statistics file detected for some load effect. Ensure that the working directory was clear and there are no other bridge output files in the folder")
            else:
                fpath = fpath[0]
            out.append(PyBtlsOutput.BtlsOutputIntervalStatistics(
                path = fpath,
                descriptor = desc_name
            ))
        return out
    
    def _read_generated_vehicles_file(self):
        return PyBtlsOutput.BtlsOutputVehicles(
                path = self.working_dir+"/Output/output_vehicle_file.txt",
                file_format = self.options.write_vehicle_file_format
            )

    def _read_block_maxima_summary_file(self):
        #There should be N number of files, where N is the number of laod effects
        N_LE = len(self.bridge.load_effect)
        out = []
        for i in range(0, N_LE):
            desc_name = "Load effect " + str(i)
            fpath = glob(self.working_dir+"/BM_S_*_Eff_" + str(i+1) + ".txt") #There should only be one file with this name
            if len(fpath)>1:
                warnings.warn("WARNING: Multiple Block Maxima Summary file detected for some load effect. Ensure that the working directory was clear and there are no other bridge output files in the folder")
            else:
                fpath = fpath[0]
            out.append(PyBtlsOutput.BtlsOutputBlockMaximaSummary(
                path = fpath,
                descriptor = desc_name
            ))
        return out
        
    def flush_files(self):
        1
        
    def _parallel_run_py_file(self):
        return subprocess.Popen(["python", self.working_dir + "/BTLS_run.py"])
        
    def _begin_parallel_run(self):
        self._check_input_completeness()
        self._create_working_dir()
        self.create_BTLSin_file()
        self.create_BTLS_bridge_files()
        self.create_BTLS_input_files()
        self.create_py_file()
        return self._parallel_run_py_file()
    
    def _end_parallel_run(self):
        self.read_outputs()
        #self.flush_files()
    
    def run(self):
        self._check_input_completeness()
        self._create_working_dir()
        self.create_BTLSin_file()
        self.create_BTLS_bridge_files()
        self.create_BTLS_input_files()
        self.create_py_file()
        self.run_py_file()
        self.read_outputs()
        #self.flush_files()
        
r"""

"""
        
class BtlsParallelWrapper():
    def __init__(self, num_subprocess = None, BTLSWrapper = None, working_dir = ".btls_working_dir"):
        self.num_subprocess = num_subprocess
        self.working_dir = working_dir
        if (not BTLSWrapper == None):
            if not isinstance(BTLSWrapper, list):
                self._BTLSWrapper = [BTLSWrapper]
            else:
                self._BTLSWrapper = BTLSWrapper
        else:
            self._BTLSWrapper = []
    
    @property
    def BTLSWrapper(self):
        return self._BTLSWrapper
    @BTLSWrapper.setter
    def BTLSWrapper(self, BTLSWrapper):
        if not isinstance(BTLSWrapper, list):
            self._BTLSWrapper = [BTLSWrapper]
        else:
            self._BTLSWrapper = BTLSWrapper
        
    def add_BTLSWrapper(self, instance):
        if not isinstance(instance, list):
            instance = [instance]
        self._BTLSWrapper.extend(instance)
        
    def add(self, instance):
        #Alias for add_BTLS_instance
        self.add_BTLSWrapper(instance)
        
    def _create_working_dir(self):
        os.makedirs("./" + self.working_dir, exist_ok=True)
        
    def _delete_working_dir(self):
        try:
            shutil.rmtree("./" + self.working_dir)
            return True
        except :
            return False
    
    def run(self):
        #In the future, should call _select_run() instead, with the entire bridges as input
        self._create_working_dir()
        num_bridge = len(self.BTLSWrapper)
        required_n_loop = int(np.ceil(num_bridge/self.num_subprocess))
        calc_performed = 0
        for i in range(0, required_n_loop):
            calc_remaining = num_bridge - calc_performed
            subprocess = []
            for k in range(0, np.minimum(self.num_subprocess, calc_remaining)):
                offset = i*self.num_subprocess
                print("Starting Kernel " + str(k+offset))
                self.BTLSWrapper[k+offset].force_set_working_dir(self.working_dir + "/" + str(k+offset))
                subprocess.append(self.BTLSWrapper[k+offset]._begin_parallel_run())
                
            #Wait for all subprocesses to finish
            for k in range(0, np.minimum(self.num_subprocess, calc_remaining)):
                subprocess[k].wait()
                
            calc_performed += self.num_subprocess
            
        #end parallel process
        #Just do normal loop here. Should be fast enough
        for i in range(0, num_bridge):
            self.BTLSWrapper[i]._end_parallel_run()
        
        #self._delete_working_dir()

    """
    CLUSTER RUN.
    For running in multiple computers with shared folder (e.g., using Syncthing, Google Drive)
    Note that folder should be synchronised fairly often. Otherwise there might be calculations overlap happening between different subprocesses
    """

    """
    New run method. In the future, run() method will be deprecated in favor of parallel_run() or begin_cluster_run()
    This is to prevent ambiguity between parallel or cluster run
    """
    #def run(self):
    #    warnings.warn("WARNING: run() method is deprecated. Currently this will run parallel on single machine. In the future, please use parallel_run() or begin_cluster_run() to prevent ambiguity")
    #    self.parallel_run()


    """
    Begin cluster run
    instantiate subprocess module according to num_subprocess specified
    """
    def begin_cluster_run(self):
        #Check if working dir already exists
        dir_state = self._check_cluster_working_dir_state()
        #If no directory, create a new directory
        if dir_state == 0:
            self._create_cluster_dir()
        
        #Is the job complete? or is there more calculations needed? Or is this a new job?
        job_state = self._check_job_state()
        #If job is done, raise exception to user
        if job_state == "done":
            raise ValueError("ERROR: there exist a cluster job that is finished already. Please end the cluster job first using end_cluster_run() method")
        elif job_state == "none":
            #There is currently no job yet. begin a new job
            self._prepare_cluster_run()
            #Then begin the cluster run for this unit
            self._cluster_run()

        elif job_state == "ongoing":
            #Help the current ongoing job
            self._cluster_run()


    """
    End the cluster run. Can only end when all jobs are done.
    Read all the outputs into the original object, save the original object, delete the working directory
    """
    def end_cluster_run():
        #Verify that cluster job is done
        job_state = self._check_job_state()
        if job_state == "none":
            raise ValueError("ERROR: There is no cluster job found in the specified working directory. Please check the working_dir of this object and verify that there is a job running")
        elif job_state == "ongoing":
            raise ValueError("ERROR: There is an ongoing job that is not finished. Please finish them first by running begin_cluster_run() method")
        
        #Cluster is done. Read all outputs
        self._read_parallel_outputs()
        #Delete the cluster job working dir
        self._force_delete_cluster_working_dir()

    def purge_cluster_run_buffer_request(self):
        #Method to purge all .request_buffer.* file in all subdirectory. Useful for when a subprocess crashed mid request
        ...

    def purge_cluster_run_working(self):
        #Method to purge all .working.* file in all subdirectory. Useful for when a subprocess crashed mid working
        ...


    def parallel_run(self, delete_working_dir = True):
        #Run on only single machine
        #using the cluster run methods, but only run on this machine. No checking will be done.
        if self._check_cluster_working_dir_state() and (delete_working_dir == False):
            raise ValueError("ERROR: Working directory already exist and delete_working_dir argument specified to False. Either delete the working directory, change the working directory, change the delete_working_dir argument to true")
        
        if (delete_working_dir == True):
            self._force_delete_cluster_working_dir()

        if not self._check_cluster_working_dir_state():
            self._create_cluster_dir()

        self._prepare_cluster_run()
        self._read_parallel_outputs()

        if (delete_working_dir == True):
            self._force_delete_cluster_working_dir()

    def save(self):
        #Save the object using pickle
        ...

    def _check_cluster_working_dir_state(self):
        #Check if the cluster job working directory exist. Return 0 if doesn't exist, 1 if exist.
        ...

    def _create_cluster_dir(self):
        #Create cluster compute directory, including necessary folder structure
        ...

    def _check_job_state(self):
        #Check the state of the current job
        #Return:
        #   "done": cluster job is done
        #   "none": no cluster job found
        #   "ongoing": there's an ongoing job ready
        ...

    def _prepare_cluster_run(self):
        #Create all the BTLS files
        ...

    def _cluster_run(self):
        #Create subprocess folder for this compute unit
        #Run the subprocess python files
        self._create_subprocess()
        subprocess = self._run_subprocess()
        subprocess.wait()
        print("Cluster run finished")

    def _create_subprocess(self):
        #Create the subprocess folder
        ...

    def _run_subprocess(self):
        #Run all the subprocess
        #Return the subprocess so the main process knows to wait
        ...

    def _read_parallel_outputs(self):
        #Read the outputs of all the parallel process
        ...

    def _force_delete_cluster_working_dir(self):
        #Forcibly delete the cluster working directory
        ...

"""
Method to import btls_parallel.obj from multiple folders. Useful when doing cluster compute.
WARNING: This method was created as a stop gap to import results for cluster computing. It WILL be deprecated and removed in the future!
Input:
    fpath: path to each subfolders for each cluster compute unit
"""
def import_btls_parallel_from_multiple_folders(fpath):
    warnings.warn("WARNING: This method was created as a stop gap to import results for cluster computing. It WILL be deprecated and removed in the future!")
    btls_parallel_list = []

    for file in fpath:
        with open(file + "/btls_parallel.obj", 'rb') as f:
            btls_parallel_list.append(pickle.load(f))

    proto_btls_parallel_obj = btls_parallel_list[0]
    for i, b in enumerate(btls_parallel_list):
        if i > 0:
            proto_btls_parallel_obj.add(b.BTLSWrapper)
    return proto_btls_parallel_obj