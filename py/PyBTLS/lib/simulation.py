from .BTLS_collections import _Vehicle, _VehClassPattern, _VehClassAxle, _VehicleBuffer
from .bridge import Bridge
from .traffic_generator import TrafficGenerator
from .traffic_loader import TrafficLoader
from .vehicle import Vehicle
from .output_config import OutputConfig
from typing import Union
import multiprocessing
import os
__all__ = ["Simulation"]


class Simulation():

    def __init__(self):
        """
        This is the class for setting and running simulations.
        """

        try:
            multiprocessing.set_start_method('spawn')
        except:
            pass

        self._sim_count = 0
        self._sim_argument = []

    def add_sim(self, bridge:Bridge=None, traffic_generator:Union[TrafficGenerator,TrafficLoader]=None, no_day:int=None, output_config:OutputConfig=None, time_step:float=0.1, min_gvw:int=0, vehicle:Vehicle=None, active_lane:list[int]=None, tag:str=None, **kwargs) -> None:
        """
        Add a simulation to the simulation queue.

        Parameters
        ----------
        bridge : Bridge, optional\n
            The bridge to be calculated load effect. If not provided, there will not be load effect calculation.

        traffic_generator : Union[TrafficGenerator,TrafficLoader], optional\n
            The traffic generator is to provide the traffics (either generated or recorded).

        no_day : int, optional\n
            The number of days to be simulated (in day). If not provided, the number of days will be the same as the recorded traffic (if given). A single-vehicle simulation will ignore this argument.

        output_config : OutputConfig, optional\n
            The output configuration. This argument is essential for traffic simulation. A single-vehicle simulation will ignore this argument.

        time_step : float, optional\n
            The calculation time step (in s) for the simulation (about precision). This argument is only used in load effect calculation for traffic simulation. A single-vehicle simulation will ignore this argument.

        min_gvw : int, optional\n
            The minimum gross vehicle weight (in kN) to be considered in the load effect calculation for traffic simulation. A single-vehicle simulation will ignore this argument.

        vehicle : Vehicle, optional\n
            The vehicle for a single-vehicle simulation.

        active_lane : list[int], optional\n
            The active bridge lanes during the simulation. The default is None, which means all lanes are active. [1-based global index].

        tag : str, optional\n
            The tag for the simulation. The default tag will be "Sim_{Simulation Order}".

        Keyword Arguments
        -----------------
        overlap_avoid_distance : float, optional\n
            The minimum chase distance (in m) between two vehicles to avoid overlap (should equal to bridge length). If the bridge argument has an input then no need to specify this argument. The default is 100.0.

        track_progress : bool, optional\n
            Whether to track the simulation progress. A single-vehicle simulation will ignore this argument. The default is False.
        """
        
        self._sim_count += 1
        if tag is None:
            sim_tag = "Sim_"+str(self._sim_count)
        else:
            sim_tag = tag

        overlap_avoid_distance = kwargs.get("min_chase_distance", 100.0)
        track_progress = kwargs.get("track_progress", False)

        self._sim_argument.append((bridge, traffic_generator, no_day, output_config, time_step, min_gvw, vehicle, active_lane, sim_tag, overlap_avoid_distance, track_progress))

    def _single_sim(self, args:tuple) -> None:

        bridge, traffic_generator, no_day, output_config, time_step, min_gvw, vehicle, active_lane, sim_tag, overlap_avoid_distance, track_progress = args

        if traffic_generator is not None:
            self._single_traffic_sim(bridge, traffic_generator, no_day, output_config, time_step, min_gvw, active_lane, sim_tag, overlap_avoid_distance, track_progress)
        elif vehicle is not None:
            self._single_vehicle_sim(bridge, vehicle, active_lane, sim_tag)
        else:
            raise ValueError("Either traffic_generator or vehicle should be provided.")
        
    def _single_vehicle_sim(self, bridge, vehicle, active_lane, sim_tag) -> None:

        os.mkdir(str(sim_tag))
        os.chdir(str(sim_tag))

        if not isinstance(bridge, Bridge):
            raise TypeError("Argument bridge needs to be Bridge type.")
        
        if not isinstance(vehicle, (Vehicle,_Vehicle)):
            raise TypeError("Argument vehicle needs to be Vehicle type.")
        
        if active_lane is None:
            lane_for_calc = list(range(1,bridge.no_lane+1))  # 1-based global index
        else:
            if not isinstance(active_lane, list):
                raise TypeError("Argument active_lane needs to be a list.")
            if max(active_lane) > bridge.no_lane:
                raise ValueError("The maximum lane index in active_lane is larger than the number of lanes on the bridge.")
            lane_for_calc = active_lane  # 1-based global index

        no_dir = 2
        bridge_length = bridge.length
        vehicle_time_gap = 2 * (bridge_length+vehicle.get_length())/1.0  # in s

        vehicle.set_velocity(1.0)  # 1 m/s

        no_lane_dir_1 = [bridge.no_lane,0]
        no_lane_dir_2 = [0,bridge.no_lane]

        output_config = OutputConfig()
        output_config.set_event_output(write_time_history=True, write_each_event=True)  # set output

        for i in range(no_dir):
            current_time = 0.0
            vehicle.set_time(current_time)

            os.mkdir("dir"+str(i+1))
            os.chdir("dir"+str(i+1))
            output_config._setRoadProperties(bridge.no_lane, 1, no_lane_dir_1[i], no_lane_dir_2[i])
            load_calc = bridge._get_bridge(output_config)  # vehicle drive from one dirn then another

            load_calc.initializeDataMgr(current_time)
            load_calc.setCalcTimeStep(0.01)
            load_calc.update(0.0, 0.0)

            for j,lane_index in enumerate(lane_for_calc):
                next_arrival_time = (j+1) * vehicle_time_gap
                vehicle.set_time(current_time)

                vehicle.set_direction(i+1)
                vehicle.set_local_from_global_lane(lane_index,bridge.no_lane)
                load_calc.addVehicle(vehicle)
                load_calc.update(next_arrival_time, current_time)
    
                current_time = next_arrival_time

            load_calc.finish()
            os.chdir("..")

        os.chdir("..")
    
    def _single_traffic_sim(self, bridge, traffic_generator, no_day, output_config, time_step, min_gvw, active_lane, sim_tag, overlap_avoid_distance, track_progress) -> None:

        os.mkdir(str(sim_tag))
        os.chdir(str(sim_tag))
        
        if isinstance(traffic_generator, TrafficGenerator) and no_day is None:
            raise ValueError("Argument no_day is not given.")
        elif isinstance(traffic_generator, TrafficLoader) and no_day is None:
            no_day = traffic_generator.sim_day
        else:
            no_day = int(no_day)
        
        if not isinstance(output_config, OutputConfig):
            raise TypeError("Argument output needs to be OutputConfig type.")
        
        output_config._setRoadProperties(traffic_generator.no_lane, traffic_generator.no_dir, traffic_generator.no_lane_dir_1, traffic_generator.no_lane_dir_2)  # this info is used by VehBuffer and EventManager(in CBridge)

        if traffic_generator.vehicle_classifier == 0:
            vehicle_classifier = _VehClassAxle()
        else:
            vehicle_classifier = _VehClassPattern()

        current_time = 0.0
        end_time = no_day * 86400.0  # 24*3600

        vehicle_buffer = _VehicleBuffer(output_config, vehicle_classifier, current_time)
        bridge_length = overlap_avoid_distance  # magic number, to avoid vehicle overlap

        if track_progress:
            current_day = 0
            sim_progress_print = ""
            print("Starting simulation...")
            print("Day complete...")

        if isinstance(bridge, Bridge):
            load_calc = bridge._get_bridge(output_config)
            if bridge.no_lane != traffic_generator.no_lane:
                raise RuntimeError("The number of lanes in the bridge and traffic generator are not equal.")
            load_calc.initializeDataMgr(current_time)
            load_calc.setCalcTimeStep(time_step)
            bridge_length = bridge.length

        if isinstance(traffic_generator, TrafficGenerator):
            lane_list = traffic_generator._get_traffic_generator(bridge_length)
        elif isinstance(traffic_generator, TrafficLoader):
            lane_list = traffic_generator._get_traffic_loader()
        else:
            raise TypeError("traffic_generator should be either TrafficGenerator or TrafficLoader.")
        
        if active_lane is None:
            lane_for_calc = lane_list
        else:
            if not isinstance(active_lane, list):
                raise TypeError("Argument active_lane needs to be a list.")
            if max(active_lane) > traffic_generator.no_lane:
                raise ValueError("The maximum lane index in active_lane is larger than the number of lanes on the bridge.")
            lane_for_calc = [lane_list[i-1] for i in active_lane]

        while current_time <= end_time:
            lane_for_calc = sorted(lane_for_calc, key=lambda t: t.getNextArrivalTime())

            next_arrival_time = lane_for_calc[0].getNextArrivalTime()
            vehicle = lane_for_calc[0].getNextVehicle()

            if vehicle == None:  # This is to skip the empty (NoneType) vehicle at the end of Read&Sim
                break

            vehicle_buffer.addVehicle(vehicle)
            if isinstance(bridge, Bridge):
                load_calc.update(next_arrival_time, current_time)
                if vehicle.get_gvw() > int(min_gvw):  # BTLS requires in size_t kN
                    load_calc.addVehicle(vehicle)

            current_time = vehicle.get_time()

            if track_progress:
                if current_time > 86400 * (current_day + 1):
                    current_day += 1
                    sim_progress_print += "\t" + str(current_day)
                    if current_day % 10 == 0:
                        print(sim_progress_print)
                        sim_progress_print = ""

        if isinstance(bridge, Bridge):
            load_calc.finish()

        vehicle_buffer.flushBuffer()

        os.chdir("..")

    def run(self, no_core:int=None) -> None:
        """
        Run the simulations.
        If no_core is one or there is only one added simulation, the running will be single-core. Otherwise, the running will be multi-core.
        By default, (no_cpu_logic_core - 2) processes will be used for multi-core running. 
        """

        if no_core == 1 or len(self._sim_argument) == 1:
            for sim_arg in self._sim_argument:
                self._single_sim(sim_arg)
        else:
            no_processes = no_core if no_core is not None else multiprocessing.cpu_count() - 2
            with multiprocessing.Pool(processes=no_processes) as pool:
                pool.map(self._single_sim, self._sim_argument)

