"""
This demo is for the case passing a single heavy vehicle over a bridge, to check the corresponding maximum load effect. 

The bridge: 
    - Has two 3.5m-width lanes. 
    - Has a length of 20m.
    - Has a width of 9m.
    - Has two load effects being considered (they are the same for simplification, mid-span BM). 

The vehicle:
    - Has 3 axles, with axle weights of 100kN, 100kN, and 100kN.
    - Has axle spacings of 3m and 7m.
    - Has axle widths of 2m, 2m, and 2m.

**Notice**: Due to Python multiprocessing, it is essential to define the simulation in a function. 
"""

import pybtls as pb
from pathlib import Path


def main():

    # set the first load effect by using a 2D influence surface
    lanes_position = [(0.5,4.0), (5.0,8.5)]  # 1m gap between lane 1 and lane 2. 
    IS_matrix = [
        [0.0, 0.0, 4.5, 9.0],
        [0.0, 0.0, 0.0, 0.0],
        [10.0, 5.0, 5.0, 5.0],
        [20.0, 0.0, 0.0, 0.0]
        ]
    load_effect_1 = pb.InfluenceSurface()
    load_effect_1.set_IS(IS_matrix, lanes_position)

    # set the second load effect by using discrete influence lines
    load_effect_2_lane_1 = pb.InfluenceLine('discrete')
    load_effect_2_lane_1.set_IL(
        position=[0.0, 10.0, 20.0], 
        ordinate=[0.0, 5.0, 0.0]
        )
    load_effect_2_lane_2 = pb.InfluenceLine('built-in')
    load_effect_2_lane_2.set_IL(
        type="1MidSpanBM",
        length=20.0
        )
    

    # set the bridge
    bridge = pb.Bridge(length=20.0, no_lane=2)
    bridge.add_load_effect(inf_line_surf=load_effect_1)
    bridge.add_load_effect(inf_line_surf=[load_effect_2_lane_1,load_effect_2_lane_2])


    # set the vehicle
    vehicle = pb.Vehicle(no_axle=3)
    vehicle.set_axle_weights([100.0, 100.0, 100.0])
    vehicle.set_axle_spacings([3.0, 7.0])
    vehicle.set_axle_widths([2.0, 2.0, 2.0])


    # set simulation
    sim_task = pb.Simulation(Path(__file__).parent)
    sim_task.add_sim(
        bridge=bridge, 
        vehicle=vehicle,
        active_lane=[1],
        tag="Case1-Lane1"
        )  # vehicle passes lane 1
    sim_task.add_sim(
        bridge=bridge, 
        vehicle=vehicle,
        active_lane=[2],
        tag="Case1-Lane2"
        )  # vehicle passes lane 2
    

    # run simulation simultaneously
    sim_task.run(no_core=2)


if __name__ == "__main__":
    main()

