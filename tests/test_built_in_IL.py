from pybtls import Vehicle, Bridge, InfluenceLine, Simulation
from typing import Literal
from pathlib import Path
import shutil
import pycba as cba
import pytest
from pybtls import Plot


def get_input(bridge_spans: list[float]):
    input_list = []

    for bridge_span in bridge_spans:
        input_list.extend(
            [
                (f"R{bridge_span}R", bridge_span / 2, "M", bridge_span, 1),
                (
                    f"R{bridge_span}R{bridge_span}R",
                    bridge_span,
                    "M",
                    2 * bridge_span,
                    2,
                ),
                (f"R{bridge_span}R", 0.0, "R", bridge_span, 3),
                (f"R{bridge_span}R", bridge_span, "R", bridge_span, 4),
                (f"R{bridge_span}R{bridge_span}R", 0.0, "R", 2 * bridge_span, 6),
                (
                    f"R{bridge_span}R{bridge_span}R",
                    2 * bridge_span,
                    "R",
                    2 * bridge_span,
                    5,
                ),
                (
                    f"R{bridge_span}R{bridge_span}R{bridge_span}R",
                    bridge_span,
                    "M",
                    3 * bridge_span,
                    8,
                ),
                (
                    f"R{bridge_span}R{bridge_span}R{bridge_span}R",
                    2 * bridge_span,
                    "M",
                    3 * bridge_span,
                    9,
                ),
            ]
        )

    return input_list


@pytest.mark.parametrize(
    "cba_config, cba_position, cba_LE, bridge_length, inf_type",
    get_input([10.0, 20.0, 30.0, 40.0]),
)
def test_built_in_IL(
    cba_config: str,
    cba_position: float,
    cba_LE: str,
    bridge_length: float,
    inf_type: Literal[1, 2, 3, 4, 5, 6, 8, 9],
):
    inf_line = InfluenceLine("built-in")
    inf_line.set_IL(type=inf_type, length=bridge_length)

    bridge = Bridge(bridge_length, 1)
    bridge.add_load_effect(inf_line)

    unit_PL = Vehicle(1)
    unit_PL.set_axle_weights([1.0])
    unit_PL.set_axle_spacings([0.0])

    out_dir = Path(__file__).parent / "temp"
    if out_dir.exists():
        shutil.rmtree(out_dir)

    sim = Simulation(out_dir)
    sim.add_sim(bridge=bridge, vehicle=unit_PL, tag="BTLS")
    sim.run()
    sim_out = sim.get_output()["BTLS"]
    BTLS_IL = sim_out.read_data("time_history")["dir1"]

    (L, EI, R, eType) = cba.parse_beam_string(cba_config)
    ils = cba.InfluenceLines(L, EI, R, eType)
    ils.create_ils(step=0.1)
    _, eta = ils.get_il(cba_position, cba_LE)

    # Plot.plot_TH(BTLS_IL)
    # import matplotlib.pyplot as plt
    # fig, axs = plt.subplots(1, 1)
    # ils.plot_il(cba_position, cba_LE, axs)

    assert pytest.approx(abs(BTLS_IL["Effect 1"]).max(), rel=1e-2) == abs(eta).max()
    assert pytest.approx(abs(BTLS_IL["Effect 1"]).min(), rel=1e-2) == abs(eta).min()
