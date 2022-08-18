import csv
import math
import rainflow


class FatigueCalculation():
    def __init__(self, Sim:bool, fatigue_file_name:str, load_effect_num:int, SN_file_name:str, SN_number:int, SN_file_log:bool, section_class:float) -> None:
        if Sim:
            pass  # do a BTLS simulation first
        self.fatigue_analysis(fatigue_file_name,load_effect_num,SN_file_name,SN_number,SN_file_log,section_class)
        
    def fatigue_analysis(self, fatigue_file_name:str, load_effect_num:int, SN_file_name:str, SN_number:int, SN_file_log:bool, section_class:float) -> None:
        # For fatigue file
        self.fatigue_loads = self.read_fatigue_file(fatigue_file_name,load_effect_num)
        self.load_to_stress()  # remember to change it
        self.fatigue_after_rainflow = self.rainflow_algorithm(self.fatigue_loads,2)
        for i in range(0,len(self.fatigue_after_rainflow)):
            self.fatigue_after_rainflow[i][0] = self.__to_common_logarithm(self.fatigue_after_rainflow[i][0])
        # For SN file
        self.SN_curves = self.read_SN_file(SN_file_name,SN_number,SN_file_log)
        self.SN_curves_in_use = self.__select_SN_curve(section_class)
        # Calculate the fatigue index 
        self.fatigue_index = self.__miners_rule(self.fatigue_after_rainflow,self.SN_curves,section_class)
        print(self.fatigue_index)

    # To read fatigue data generated by BTLS
    def read_fatigue_file(self, file_name:str, load_effect_num:int) -> list:
        fatigue_loads = []
        with open(file_name, "r") as file:
            for data in file.readlines():
                fatigue_loads.append(float(data.split()[2*load_effect_num]))
        return fatigue_loads

    # To convert from load to stress then input to rainflow
    def load_to_stress(self):
        pass

    # To apply the rainflow algorithm to stress
    def rainflow_algorithm(self, fatigue_stress:list, ndigits:int) -> list:
        fatigue_after_rainflow = rainflow.count_cycles(fatigue_stress,ndigits)
        for i in range(0,len(fatigue_after_rainflow)):
            fatigue_after_rainflow[i] = list(fatigue_after_rainflow[i])
        return fatigue_after_rainflow

    # To convert base number to logarithm
    def __to_common_logarithm(self, input_num):
        return math.log10(input_num)

    # To read user-defined SN curve file
    def read_SN_file(self, file_name:str, SN_number:int, logarithm:bool) -> list:
        csv_reader = csv.reader(open(file_name))
        self.SN_number = SN_number  # number of curves defined
        SN_curves = []
        for i in range(2*SN_number):
            SN_curves.append([])
        for line in csv_reader:
            for i in range(0,2*SN_number):
                if logarithm:
                    SN_curves[i].append(float(line[i]))
                else:
                    SN_curves[i].append(self.__to_common_logarithm(float(line[i])))
        return SN_curves

    # To determine which interval the data drops in (The input list should be in descending order)
    def __interval_check(self, target_value, exist_intervals:list):
        if target_value in exist_intervals:
            return [exist_intervals.index(target_value)]
        else:
            res = len(exist_intervals)-1
            if target_value < exist_intervals[-1]:
                return [-1,-1]  # lower than the minimum value
            while res>=0 and target_value>=exist_intervals[res]:
                res -= 1
            return [res,res+1]

    # To determine which (two) curve(s) will be used, and section_class=1 is the greatest
    def __select_SN_curve(self, section_class:float):
        target_value = self.SN_number-section_class
        exist_intervals = list(range(0,self.SN_number))
        exist_intervals.reverse()
        self.__SN_curve_selection = self.__interval_check(target_value,exist_intervals)
        if -1 in self.__SN_curve_selection:
            raise RuntimeError("The section class has exceeded the range of SN curves!")
        return self.SN_curves[2*self.__SN_curve_selection[0]:2*self.__SN_curve_selection[-1]+2]  # Note this +2 not +1 is because Python's "list[0:n+1] to get n+1 elements" grammer...
 
    # To determine which interval the stress drops in (The input list should be in descending order)
    def __SN_interval_check(self, target_value, exist_intervals):
        return_value = self.__interval_check(target_value,exist_intervals)
        if return_value.count(-1) == 2:
            print("Stress is not counted as its amplitude is less than the smallest SN curve point!")
        elif return_value.count(-1) == 1:
            raise RuntimeError("Stress amplitude is larger than the largest SN curve point!")
        return return_value

    # To calcualte the specific value within the interval based on linear interpolation
    def __SN_linear_interpolation(self, x_data:list, y_data:list, y_value:float) -> float:
        SN_interval_selection = self.__SN_interval_check(y_value,y_data)
        if SN_interval_selection.count(-1) == 2:
            return float('inf')
        if len(SN_interval_selection) == 1:
            x_value = (x_data[SN_interval_selection[-1]]+x_data[SN_interval_selection[-1]])/2
            return x_value
        else:
            x_value = (y_value-y_data[SN_interval_selection[0]])/(y_data[SN_interval_selection[-1]]-y_data[SN_interval_selection[0]])*(x_data[SN_interval_selection[-1]]-x_data[SN_interval_selection[0]]) + x_data[SN_interval_selection[0]]
            return x_value

    # To use Miner's rule for fatigue_index
    def __miners_rule(self, fatigue_loads:list, SN_curves:list, section_class:float) -> float:
        fatigue_index = 0.0
        for fatigue_load in fatigue_loads:
            fatigue_life = []
            for SN_curve in [[SN_curves[0],SN_curves[1]],[SN_curves[-2],SN_curves[-1]]]:
                fatigue_life.append(self.__SN_linear_interpolation(SN_curve[0],SN_curve[1],fatigue_load[0]))
            if math.isinf(fatigue_life[0]) or math.isinf(fatigue_life[1]):
                fatigue_life = float('inf')
            elif not (math.isinf(fatigue_life[0]) or math.isinf(fatigue_life[1])) and fatigue_life[0] == fatigue_life[1]:
                fatigue_life = (fatigue_life[0]+fatigue_life[1])/2
            else:
                fatigue_life = fatigue_life[0] - (section_class-1-self.__SN_curve_selection[0])/(self.__SN_curve_selection[-1]-self.__SN_curve_selection[0])*(fatigue_life[0]-fatigue_life[1])
            fatigue_index += fatigue_load[1]/(10**fatigue_life)
        return fatigue_index


if __name__ == '__main__':
    fatigue_4_6 = FatigueCalculation(False,"BL_4.6_Fatigue.txt",1,"SN_4.6_Eff1.csv",2,False,1.5) 


# My/Z FS/bI

# 目前来看BTLS当前产生的Fatigue数据已经足够好
# 读数据
# 运行rainflow算法
# 读R-N曲线，需要完成力到应力的转化和不同荷载影响的合并
# 得出系数


# 给了文件夹之后对疲劳相关的文件自动分析?