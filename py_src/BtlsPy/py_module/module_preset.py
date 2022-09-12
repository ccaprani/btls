from BtlsPy.c_module import CConfigData


class Settings:
    def __init__(self) -> None:
        pass
    
    @classmethod
    def gen_and_sim(cls, bridge_file:str, lane_file:str, traffic_folder:str, no_days:int, buffer_size:int=100000, infline_file:str="", infsurf_file:str="") -> dict:
        if len(infline_file) + len(infsurf_file) == 0:
            raise ValueError("Must give influence line of surface!")
        road_setting = CConfigData.Road_Config(lane_file,1,1,2,2)
        gen_setting = CConfigData.Gen_Config(traffic_folder,True,no_days,190.0,0.0,100.0)
        traffic_setting = CConfigData.Traffic_Config(2,6,1,5.0,30.0,5.0,0.05)
        sim_setting = CConfigData.Sim_Config(True,bridge_file,infline_file,infsurf_file,0.1,0)
        output_request = input("Please request the output files (in a list): ['BlockMax','POT','Stats','Fatigue'].")
        output_setting = cls.write_output(output_request, buffer_size)
        settings_return = {"road_setting":road_setting,"gen_setting":gen_setting,"traffic_setting":traffic_setting,"sim_setting":sim_setting,"output_setting":output_setting}
        return settings_return

    @classmethod
    def gen_only(cls, lane_file:str, traffic_folder:str, no_days:int, buffer_size:int=100000) -> dict:
        road_setting = CConfigData.Road_Config(lane_file,1,1,2,2)
        gen_setting = CConfigData.Gen_Config(traffic_folder,True,no_days,190.0,0.0,100.0)
        traffic_setting = CConfigData.Traffic_Config(2,6,1,5.0,30.0,5.0,0.05)
        settings_return = {"road_setting":road_setting,"gen_setting":gen_setting,"traffic_setting":traffic_setting}
        return settings_return

    @classmethod
    def read_and_sim(cls, bridge_file:str, traffic_file:str, file_format:int, use_constant_speed:bool=False, use_ave_speed:bool=False, garage_file:str="", kernal_file:str="", buffer_size:int=100000, infline_file:str="", infsurf_file:str="") -> dict:
        if len(garage_file) + len(kernal_file) == 0:
            raise ValueError("Must give garage_file or kernal_file!")
        if use_constant_speed == use_ave_speed:
            raise ValueError("Use either constant speed or average speed!")
        if len(infline_file) + len(infsurf_file) == 0:
            raise ValueError("Must give influence line of surface!")
        const_speed = 0.0
        if use_constant_speed:
            const_speed = float(input("Please input the constant speed (km/h): "))
        read_setting = CConfigData.Read_Config(True,traffic_file,garage_file,kernal_file,file_format,use_constant_speed,use_ave_speed,const_speed)
        traffic_setting = CConfigData.Traffic_Config(2,6,1,5.0,30.0,5.0,0.05)
        sim_setting = CConfigData.Sim_Config(True,bridge_file,infline_file,infsurf_file,0.1,0)
        output_request = input("Please request the output files (in a list): ['BlockMax','POT','Stats','Fatigue'].")
        output_setting = cls.write_output(output_request, buffer_size)
        settings_return = {"read_setting":read_setting,"traffic_setting":traffic_setting,"sim_setting":sim_setting,"output_setting":output_setting}
        return settings_return

    @classmethod
    def write_output(cls, output_request:list, buffer_size:int=100000):
        output_setting_VehicleFile = CConfigData.Output_Config.VehicleFile_Config(False,4,"vehicles.txt",buffer_size,True)
        output_setting_BlockMax = CConfigData.Output_Config.BlockMax_Config(False,True,True,True,1,0,buffer_size)
        output_setting_POT = CConfigData.Output_Config.POT_Config(False,True,True,True,1,0,buffer_size)
        output_setting_Stats = CConfigData.Output_Config.Stats_Config(False,True,True,3600,buffer_size)
        if 'BlockMax' in output_request:
            output_setting_BlockMax.WRITE_BM = True
        if 'POT' in output_request:
            output_setting_POT.WRITE_POT = True
        if 'Stats' in output_request:
            output_setting_Stats.WRITE_STATS = True
        output_setting = CConfigData.Output_Config(False,False,buffer_size,False,False,output_setting_VehicleFile,output_setting_BlockMax,output_setting_POT,output_setting_Stats)
        if 'Fatigue' in output_request:
            output_setting.WRITE_TIME_HISTORY = True
            output_setting.DO_FATIGUE_RAINFLOW = True
        return output_setting
