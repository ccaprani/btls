import pickle
import subprocess
import warnings
import os
import shutil
from glob import glob
from PyBTLS.py_module.parallel.options import BTLSWrapperOptions
from PyBTLS.py_module.output.output import _BtlsWrapper_DefaultOutputs
        
"""
Wrapper class for BTLS. Different to the BTLS object from PyBTLS. This class create a file that will then run the PyBTLS.BTLS object.
   
"""
class BTLSWrapper():
    def __init__(self, bridge = None, bridge_flow = None, working_dir = ".btls_default_working_dir", num_days = 1):
        self._working_dir = working_dir
        self.options = BTLSWrapperOptions(num_days)
        
        self.bridge = bridge
        self.bridge_flow = bridge_flow
        
        #Input files
        self.traffic = None
        self.garage = None
        self.kernels = None
        self.constant_vehicle = None
        
        self._traffic_input = None
        self._traffic_input_file = None
        self._traffic_input_file_format = None  # Traffic input file format (CASTOR - 1, BeDIT - 2, DITIS - 3, MON - 4):
        
        self._outputs = _BtlsWrapper_DefaultOutputs()
        
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
            if (self.traffic is None):
                warnings.warn("'BtlsTraffic' required for 'Grave' vehicle_generation_model")
                return False
        elif self.options.vehicle_generation_model == "Constant":
            if (self.constant_vehicle is None):
                warnings.warn("'BtlsConstantVehicle' required for 'Constant' vehicle_generation_model")
                return False
        elif self.options.vehicle_generation_model == "Garage":
            if (self.garage is None) or (self.kernels is None):
                warnings.warn("'BtlsGarage' and 'BtlsKernels' required for 'Garage' vehicle_generation_model")
                return False
                
        if self.bridge_flow is None:
            warnings.warn("'BtlsBridgeFlow' required")
            return False
        
        if self.options.program_mode == "Read & Sim":
            if self._traffic_input is None:
                warnings.warn("pregenerated traffic input file required for 'Read & Sim' program_mode. Set using traffic_input property")
                return False
            if self.traffic_input_file_format is None:
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
        BTLSin_txt += "No garage file\n" if self.garage is None else "./Input/garage.txt\n"
        BTLSin_txt += "No kernels file\n" if self.kernels is None else "./Input/kernels.csv\n"
        BTLSin_txt += "No constant_vehicle file\n" if self.constant_vehicle is None else "./Input/constant_vehicle.csv\n"
        BTLSin_txt += "No traffic input file\n" if self._traffic_input is None else "./Input/traffic_input_file.txt\n"
        BTLSin_txt += "2\n" if self._traffic_input_file_format is None else str(self._traffic_input_file_format) + "\n"
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
        self.bridge._create_txt()
        with open(self.working_dir + "/Input/bridge.txt", 'w') as f:
            f.write(self.bridge._get_bridge_txt())
        with open(self.working_dir + "/Input/IL.txt", 'w') as f:
            f.write(self.bridge._get_il_txt())
    
    def create_BTLS_input_files(self):
        if not self.constant_vehicle is None:
            self.constant_vehicle.write_as_csv(self.working_dir + "/Input/constant_vehicle.csv")
        if not self.bridge_flow is None:
            self.bridge_flow.write_as_csv(self.working_dir + "/Input/LaneFlowData.csv")
        if not self.traffic is None:
            self.traffic.write_as_csv(self.working_dir + "/Traffic")
        if not self.garage is None:
            self.garage.write_as_txt(path = self.working_dir + "/Input/garage.txt")
        if not self.kernels is None:
            self.kernels.write_as_csv(path = self.working_dir + "/Input/kernels.txt")
        
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
        self.flush_files()
    
    def run(self):
        self._check_input_completeness()
        self._create_working_dir()
        self.create_BTLSin_file()
        self.create_BTLS_bridge_files()
        self.create_BTLS_input_files()
        self.create_py_file()
        self.run_py_file()
        self.read_outputs()
        self.flush_files()

    def load(self, fname):
        with open(fname, 'rb') as f:
            self.__dict__.update(pickle.load(f).__dict__)

    def save(self, fname):
        with open(fname, 'wb') as f:
            pickle.dump(self, f)

