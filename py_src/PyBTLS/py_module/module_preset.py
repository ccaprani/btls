from PyBTLS.c_module import ConfigData


class Settings:
    def __init__(self) -> None:
        pass
    
    @classmethod
    def gen_only(cls, lane_file:str, traffic_folder:str, no_days:int, file_format:int = 10, garage_file:str="", kernal_file:str="", constant_file:str="") -> dict:
        road_config = ConfigData.Road_Config(lane_file,1,1,2,2)
        gen_config = ConfigData.Gen_Config(traffic_folder,True,no_days,190.0,0.0,100.0)
        traffic_config = ConfigData.Traffic_Config(1,6,1,5.0,30.0,5.0,0.05)
        if traffic_config.VEHICLE_MODEL == 1 and len(constant_file) == 0:
            raise ValueError("Must give constant_file!")
        if traffic_config.VEHICLE_MODEL == 2 and (len(garage_file) == 0 or len(kernal_file) == 0) and file_format == 10:
            raise ValueError("Must give garage_file, kernal_file, and file_format!")
        read_config = ConfigData.Read_Config(False,"",garage_file,kernal_file,constant_file,file_format,False,False,0.0)
        configs_return = {"road_config":road_config,"gen_config":gen_config,"traffic_config":traffic_config,"read_config":read_config}
        return configs_return

    @classmethod
    def gen_and_sim(cls, bridge_file:str, lane_file:str, traffic_folder:str, no_days:int, file_format:int = 10, garage_file:str="", kernal_file:str="", constant_file:str="", buffer_size:int=100000, infline_file:str="", infsurf_file:str="", BlockMax:bool = False, POT:bool = False, Stats:bool = False, Fatigue:bool = False) -> dict:
        configs_return = cls.gen_only(lane_file,traffic_folder,no_days,file_format,garage_file,kernal_file,constant_file)
        if len(infline_file) + len(infsurf_file) == 0:
            raise ValueError("Must give influence line or surface!")
        sim_config = ConfigData.Sim_Config(True,bridge_file,infline_file,infsurf_file,0.1,0)
        output_config = cls.write_output(BlockMax, POT, Stats, Fatigue, buffer_size)
        configs_return.update({"sim_config":sim_config,"output_config":output_config})
        return configs_return

    @classmethod
    def read_and_sim(cls, bridge_file:str, traffic_file:str, file_format:int, use_constant_speed:bool=False, use_ave_speed:bool=False, buffer_size:int=100000, infline_file:str="", infsurf_file:str="", BlockMax:bool = False, POT:bool = False, Stats:bool = False, Fatigue:bool = False) -> dict:
        const_speed = 0.0
        if use_constant_speed and not use_ave_speed:
            const_speed = float(input("Please input the constant speed (km/h): "))
        if len(infline_file) + len(infsurf_file) == 0:
            raise ValueError("Must give influence line or surface!")
        read_config = ConfigData.Read_Config(True,traffic_file,"","","",file_format,use_constant_speed,use_ave_speed,const_speed)
        sim_config = ConfigData.Sim_Config(True,bridge_file,infline_file,infsurf_file,0.1,0)
        output_config = cls.write_output(BlockMax, POT, Stats, Fatigue, buffer_size)
        configs_return = {"read_config":read_config,"sim_config":sim_config,"output_config":output_config}
        return configs_return

    @classmethod
    def write_output(cls, BlockMax:bool = False, POT:bool = False, Stats:bool = False, Fatigue:bool = False, buffer_size:int=100000):
        output_config_VehicleFile = ConfigData.Output_Config.VehicleFile_Config(False,4,"vehicles.txt",buffer_size,True)
        output_config_BlockMax = ConfigData.Output_Config.BlockMax_Config(False,True,True,True,1,0,buffer_size)
        output_config_POT = ConfigData.Output_Config.POT_Config(False,True,True,True,1,0,buffer_size)
        output_config_Stats = ConfigData.Output_Config.Stats_Config(False,True,True,3600,buffer_size)
        output_config_Fatigue = ConfigData.Output_Config.Fatigue_Config(False,3,0.0,buffer_size)
        if BlockMax:
            output_config_BlockMax.WRITE_BM = True
        if POT:
            output_config_POT.WRITE_POT = True
        if Stats:
            output_config_Stats.WRITE_STATS = True
        if Fatigue:
            output_config_Fatigue.DO_FATIGUE_RAINFLOW = True
        output_config = ConfigData.Output_Config(False,False,buffer_size,False,output_config_VehicleFile,output_config_BlockMax,output_config_POT,output_config_Stats,output_config_Fatigue)
        return output_config