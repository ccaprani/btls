import pytest
from pybtls import Bridge, InfluenceLine, InfluenceSurface, OutputConfig
from pybtls.lib.BTLS import _Bridge


def get_IL_IS(bridge_span: float):
    inf_line_1 = InfluenceLine("discrete")
    inf_line_1.set_IL(
        position=[0.0, bridge_span / 2, bridge_span],
        ordinate=[0.0, bridge_span / 4, 0.0],
    )

    inf_line_2 = InfluenceLine("built-in")
    inf_line_2.set_IL(type=1, length=bridge_span)

    inf_surface = InfluenceSurface()
    lanes_position = [(0.5, 4.0), (5.0, 8.5)]
    IS_matrix = [
        [0.0, 0.0, 4.5, 9.0],
        [0.0, 0.0, 0.0, 0.0],
        [bridge_span / 2, bridge_span / 4, bridge_span / 4, bridge_span / 4],
        [bridge_span, 0.0, 0.0, 0.0],
    ]
    inf_surface.set_IS(IS_matrix, lanes_position)

    return inf_line_1, inf_line_2, inf_surface


def test_bridge_initialization():
    bridge = Bridge(length=50.0, no_lane=2)
    assert bridge.length == 50.0
    assert bridge.no_lane == 2


def test_add_load_effect_with_influence_line():
    bridge = Bridge(length=20.0, no_lane=2)
    inf_line = get_IL_IS(20.0)[0]
    bridge.add_load_effect(inf_line)
    assert bridge._no_load_effect == 1
    assert len(bridge._inf_file_dict[str(bridge._no_load_effect)]["inf_line"]) == 2
    assert bridge._inf_file_dict[str(bridge._no_load_effect)]["weight"] == [1.0, 1.0]


def test_add_load_effect_with_influence_line_list():
    bridge = Bridge(length=20.0, no_lane=2)
    inf_lines = list(get_IL_IS(20.0)[:2])
    bridge.add_load_effect(inf_lines, [0.5, 0.5])
    assert bridge._no_load_effect == 1
    assert len(bridge._inf_file_dict[str(bridge._no_load_effect)]["inf_line"]) == 2
    assert bridge._inf_file_dict[str(bridge._no_load_effect)]["weight"] == [0.5, 0.5]


def test_add_load_effect_with_influence_surface():
    bridge = Bridge(length=50.0, no_lane=2)
    inf_surface = get_IL_IS(50.0)[2]
    bridge.add_load_effect(inf_surface)
    assert bridge._no_load_effect == 1
    assert len(bridge._inf_file_dict[str(bridge._no_load_effect)]["inf_line"]) == 2
    assert bridge._inf_file_dict[str(bridge._no_load_effect)]["weight"] == [1.0, 1.0]


def test_get_bridge():
    bridge = Bridge(length=50.0, no_lane=2)
    inf_line_1, inf_line_2, inf_surface = get_IL_IS(50.0)
    bridge.add_load_effect(inf_line_1)
    bridge.add_load_effect(inf_line_2)
    bridge.add_load_effect(inf_surface)
    output_config = OutputConfig()
    c_bridge = bridge._get_bridge(output_config)
    assert isinstance(c_bridge, _Bridge)
