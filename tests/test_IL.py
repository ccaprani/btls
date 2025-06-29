import pybtls as pb
import pytest
from pathlib import Path
from utils import remove_folder, get_cba_IL


@pytest.mark.parametrize(
    "built_in_id, built_in_length, cba_beam_string, cba_beam_position, cba_beam_LE",
    [
        (1, 10.0, "P10R", 5.0, "M"),
        (2, 20.0, "P10R10R", 10.0, "M"),
        (3, 10.0, "P10R", 0.0, "R"),
        (4, 10.0, "P10R", 10.0, "R"),
        (5, 20.0, "P10R10R", 0.0, "R"),
        (6, 20.0, "P10R10R", 20.0, "R"),
        (8, 30.0, "P10R10R10R", 10.0, "M"),
        (9, 30.0, "P10R10R10R", 20.0, "M"),
    ],
)
def test_built_in_IL(
    built_in_id: int,
    built_in_length: float,
    cba_beam_string: str,
    cba_beam_position: float,
    cba_beam_LE: str,
):

    # A unit load
    vehicle = pb.Vehicle(no_axles=1)
    vehicle.set_axle_weights([1.0])
    vehicle.set_axle_spacings([0.0])
    vehicle.set_axle_widths([2.0])

    # Influence line built-in
    IL_built_in = pb.InfluenceLine(IL_type="built-in")
    IL_built_in.set_IL(id=built_in_id, length=built_in_length)

    # Influence line pycba
    IL_cba = pb.InfluenceLine(IL_type="discrete")
    position, ordinate = get_cba_IL(cba_beam_string, cba_beam_position, cba_beam_LE)
    (
        IL_cba.set_IL(position=position, ordinate=-ordinate)
        if built_in_id in [2, 8, 9]
        else IL_cba.set_IL(position=position, ordinate=ordinate)
    )

    # Bridge
    bridge = pb.Bridge(length=built_in_length, no_lane=1)
    bridge.add_load_effect(inf_line_surf=IL_built_in)
    bridge.add_load_effect(inf_line_surf=IL_cba)

    # Simulate
    remove_folder(Path("./temp_data"))
    sim_task = pb.Simulation(output_dir=Path("./temp_data"))
    sim_task.add_sim(bridge=bridge, vehicle=vehicle, tag="test_built_in_IL")
    sim_task.run()

    # Compare the results
    sim_output = sim_task.get_output()["test_built_in_IL"]
    time_history_data = sim_output.read_data("time_history")["dir1"]
    assert (
        (
            time_history_data["Effect 1"][1:-1] - time_history_data["Effect 2"][1:-1]
        ).abs()
        <= 0.1
    ).all()

    # Remove the temporary data folder
    remove_folder(Path("./temp_data"))


def test_compress_IL():

    # A unit load
    vehicle = pb.Vehicle(no_axles=1)
    vehicle.set_axle_weights([1.0])
    vehicle.set_axle_spacings([0.0])
    vehicle.set_axle_widths([2.0])

    # Influence line built-in
    IL_built_in = pb.InfluenceLine(IL_type="built-in")
    IL_built_in.set_IL(id=8, length=30.0)

    # Influence line pycba
    IL_cba = pb.InfluenceLine(IL_type="discrete")
    position, ordinate = get_cba_IL("P10R10R10R", 10.0, "M", resolution=0.01)
    IL_cba.set_IL(position=position, ordinate=-ordinate, compress_tolerance=0.1)

    # Bridge
    bridge = pb.Bridge(length=30.0, no_lane=1)
    bridge.add_load_effect(inf_line_surf=IL_built_in)
    bridge.add_load_effect(inf_line_surf=IL_cba)

    # Simulate
    remove_folder(Path("./temp_data"))
    sim_task = pb.Simulation(output_dir=Path("./temp_data"))
    sim_task.add_sim(bridge=bridge, vehicle=vehicle, tag="test_discrete_IL_compress")
    sim_task.run()

    # Compare the results
    sim_output = sim_task.get_output()["test_discrete_IL_compress"]
    time_history_data = sim_output.read_data("time_history")["dir1"]
    assert (
        (time_history_data["Effect 1"] - time_history_data["Effect 2"]).abs() <= 0.1
    ).all()

    # Remove the temporary data folder
    remove_folder(Path("./temp_data"))
