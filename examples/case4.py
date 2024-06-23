"""
This demo is for the case reading a traffic flow and passing them over a bridge. 

The bridge: 
    - Has four 3.5m-width lanes. 
    - Has a length of 20m.
    - Has a width of 16m.
    - Has one load effect being considered. 

The traffic flow:
    - It is from a recorded traffic file. 

**Notice**: Due to Python multiprocessing, it is essential to define the simulation in a function. 
"""

import pybtls as pb
from pathlib import Path


def main():
    # set the load effect by using a 2D influence surface
    lanes_position = [(0.5, 4.0), (4.0, 7.5), (8.5, 12.0), (12.0, 15.5)]
    IS_matrix = [
        [0.0, 0.0, 4.0, 8.0, 12.0, 16.0],
        [0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
        [10.0, 0.0, 2.5, 5.0, 2.5, 0.0],
        [20.0, 0.0, 0.0, 0.0, 0.0, 0.0],
    ]
    load_effect = pb.InfluenceSurface()
    load_effect.set_IS(IS_matrix, lanes_position)

    # set the bridge
    bridge = pb.Bridge(length=20.0, no_lane=4)
    bridge.add_load_effect(inf_line_surf=load_effect, threshold=0.0)

    # or, use traffic loader
    traffic_gen = pb.TrafficLoader(no_lane=4)
    traffic_gen.add_traffic(
        str(Path(__file__).parent / "test_traffic_file.txt"),
        file_format="MON",
        use_average_speed=False,
        use_const_speed=False,
        const_speed_value=36.0,
    )

    # set output, which will be written to HDD (here we output all possible results)
    output_config = pb.OutputConfig()
    output_config.set_event_output(write_time_history=True, write_each_event=True)
    output_config.set_vehicle_file_output(
        write_vehicle_file=True,
        vehicle_file_format=4,
        vehicle_file_name="proof_traffic_file.txt",
    )  # you will see the proof_traffic_file.txt is the same as the test_traffic_file.txt
    output_config.set_BM_output(
        write_vehicle=True, write_summary=True, write_mixed=True
    )
    output_config.set_POT_output(
        write_vehicle=True, write_summary=True, write_counter=True
    )
    output_config.set_stats_output(
        write_flow_stats=True, write_overall=True, write_intervals=True
    )
    output_config.set_fatigue_output(
        write_fatigue_event=True, write_rainflow_output=True
    )

    # set simulation
    sim_task = pb.Simulation(Path(__file__).parent)
    sim_task.add_sim(
        bridge=bridge,
        traffic=traffic_gen,
        output_config=output_config,
        time_step=0.1,
        min_gvw=35,
        # active_lane=[1,2,3,4],  # optional, if not set, all lanes will be active.
        # track_progress=False,  # optional, if True, the progress print will show up.
        tag="Case4",
    )

    # run simulation
    sim_task.run(no_core=1)

    # print(type(sim_task.get_output()[0].get_summary()["time_history"][0]))


if __name__ == "__main__":
    main()
