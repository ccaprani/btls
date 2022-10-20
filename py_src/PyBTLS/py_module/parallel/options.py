

"""

"""

class BTLSWrapperOptions():
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
        self._headway_model = 6
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
        self.write_each_loading_event_buffer_size = int(1e7)
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
        self._write_vehicle_file_format = 4
        # Vehicle file name
        #./Output/Vehicle File.txt
        # Vehicle file buffer size
        self.write_vehicle_file_buffer_size = int(1e6)
        # Write vehicle file flow statistics (1 or 0)
        self.write_vehicle_flow_statistics = 0
        
        # Analyse for Block Max (overrides remaining params) (1 or 0)
        self.analyse_block_max = 0
        # Block size for maxima (days):
        self.size_block_max_in_days = 1
        # Block size for maxima (seconds):
        self.size_block_max_in_seconds = 0
        # Write block max separated vehicle files (1 or 0):
        self.write_block_max_separated_vehicle_files = 0
        # Write block max summary files (1 or 0):
        self.write_block_max_summary_files = 0
        # Do and write block max mixed vehicle analysis (1 or 0):
        self.write_block_max_mixed_analysis = 0
        # Write block max buffer size:
        self.write_block_max_buffer_size = 250
        
        # Analyse for POT (overrides remaining params) (1 or 0)
        self.analyse_pot = 0
        # Write POT vehicle files (1 or 0):
        self.write_pot_vehicle_files = 0
        # Write POT summary files (1 or 0):
        self.write_pot_summary_files = 0
        # Write POT counter files (1 or 0):
        self.write_pot_counter_files = 0
        # POT counter size (days):
        self.pot_counter_size_in_days = 0
        # POT counter (seconds):
        self.pot_counter_size_in_seconds = 0
        # Write POT buffer size:
        self.write_pot_buffer_size = int(1e7)
        
        # Analyse for Statistics (overrides remaining params) (1 or 0)
        self.analyse_statistics = 0
        # Write cumulative statistics file (1 or 0)
        self.write_cumulative_statistics_file = 0
        # Write statistics at intervals files (1 or 0)
        self.write_interval_statistics_files = 0
        # Interval size for statistics output (seconds)
        self.interval_statistics_size = 24*3600
        # Write interval statistics buffer size:
        self.interval_statistics_buffer_size = int(1e7)
    
    @property
    def group_analyse_block_maxima(self):
        ...
    
    @group_analyse_block_maxima.setter
    def group_analyse_block_maxima(self, state):
        if state:
            state = 1
        else:
            state = 0
        self.analyse_block_max = state
        self.write_block_max_separated_vehicle_files = state
        self.write_block_max_summary_files = state
        self.write_block_max_mixed_analysis = state

    @property
    def group_analyse_pot(self):
        ...
    
    @group_analyse_pot.setter
    def group_analyse_pot(self, state):
        if state:
            state = 1
        else:
            state = 0
        self.analyse_pot = state
        self.write_pot_summary_files = state
        self.write_pot_counter_files = state

    @property
    def group_analyse_statistics(self):
        ...
    
    @group_analyse_statistics.setter
    def group_analyse_statistics(self, state):
        if state:
            state = 1
        else:
            state = 0
        self.analyse_statistics = state
        self.write_cumulative_statistics_file = state
        self.write_interval_statistics_files = state
        
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

    def print(self):
        print(self.__str__())

    def __str__(self):
        qualname = type(self).__qualname__
        out = f"PyBTLS {qualname} object at {hex(id(self))}\n" +\
                "{\n" +\
                f"  program_mode: {self.program_mode},\n" +\
                f"  num_days: {self.num_days},\n" +\
                f"  track_width: {self.track_width},\n" +\
                f"  eccentricity_std_dev: {self.eccentricity_std_dev},\n" +\
                f"  vehicle_generation_model: {self.vehicle_generation_model},\n" +\
                f"  headway_model: {self.headway_model},\n" +\
                f"  traffic_classification: {self.traffic_classification},\n" +\
                f"  nom_congested_spacing: {self.nom_congested_spacing},\n" +\
                f"  congested_speed: {self.congested_speed},\n" +\
                f"  congested_gaps_cov: {self.congested_gaps_cov},\n\n" +\
                f"  impose_constant_speed: {self.impose_constant_speed},\n" +\
                f"  use_avg_speed_if_impose_constant_speed: {self.use_avg_speed_if_impose_constant_speed},\n" +\
                f"  constant_speed_if_not_use_avg_speed: {self.constant_speed_if_not_use_avg_speed},\n\n" +\
                f"  time_step: {self.time_step},\n" +\
                f"  min_GVW: {self.min_GVW},\n\n" +\
                f"  write_full_time_history: {self.write_full_time_history},\n" +\
                f"  write_each_loading_event: {self.write_each_loading_event},\n" +\
                f"  write_each_loading_event_buffer_size: {self.write_each_loading_event_buffer_size},\n" +\
                f"  write_fatigue_event: {self.write_fatigue_event},\n" +\
                f"  write_rainflow_algorithm: {self.write_rainflow_algorithm},\n" +\
                f"  num_decimal_rainflow_algorithm: {self.num_decimal_rainflow_algorithm},\n" +\
                f"  fatigue_cutoff_value: {self.fatigue_cutoff_value},\n\n" +\
                f"  write_vehicle_file: {self.write_vehicle_file},\n" +\
                f"  write_vehicle_file_format: {self.write_vehicle_file_format},\n" +\
                f"  write_vehicle_file_buffer_size: {self.write_vehicle_file_buffer_size},\n" +\
                f"  write_vehicle_flow_statistics: {self.write_vehicle_flow_statistics},\n\n" +\
                f"  analyse_block_max: {self.analyse_block_max},\n" +\
                f"  size_block_max_in_days: {self.size_block_max_in_days},\n" +\
                f"  size_block_max_in_seconds: {self.size_block_max_in_seconds},\n" +\
                f"  write_block_max_separated_vehicle_files: {self.write_block_max_separated_vehicle_files},\n" +\
                f"  write_block_max_summary_files: {self.write_block_max_summary_files},\n" +\
                f"  write_block_max_mixed_analysis: {self.write_block_max_mixed_analysis},\n" +\
                f"  write_block_max_buffer_size: {self.write_block_max_buffer_size},\n\n" +\
                f"  analyse_pot: {self.analyse_pot},\n" +\
                f"  write_pot_vehicle_files: {self.write_pot_vehicle_files},\n" +\
                f"  write_pot_summary_files: {self.write_pot_summary_files},\n" +\
                f"  write_pot_counter_files: {self.write_pot_counter_files},\n" +\
                f"  pot_counter_size_in_days: {self.pot_counter_size_in_days},\n" +\
                f"  pot_counter_size_in_seconds: {self.pot_counter_size_in_seconds},\n" +\
                f"  write_pot_buffer_size: {self.write_pot_buffer_size},\n\n" +\
                f"  analyse_statistics: {self.analyse_statistics},\n" +\
                f"  write_cumulative_statistics_file: {self.write_cumulative_statistics_file},\n" +\
                f"  write_interval_statistics_files: {self.write_interval_statistics_files},\n" +\
                f"  interval_statistics_size: {self.interval_statistics_size},\n" +\
                f"  interval_statistics_size: {self.interval_statistics_size},\n" +\
                f"  interval_statistics_buffer_size: {self.interval_statistics_buffer_size},\n" +\
                "}"

        return out
