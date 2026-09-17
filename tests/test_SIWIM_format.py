from pathlib import Path

import pytest
import pybtls as pb


KG_TO_KN = 9.81 / 1000.0


def test_read_siwim_garage_file():
    vehicles = pb.garage.read_garage_file(
        Path(__file__).parent / "test_data/siwim.csv", garage_format=5
    )

    assert len(vehicles) == 3

    v0 = vehicles[0]
    assert v0.get_no_axles() == 9
    assert v0.get_local_lane() == 1
    assert v0.get_direction() == 1
    assert v0.get_velocity() == pytest.approx(18.9046)
    assert v0.get_gvw() == pytest.approx(337.617)
    assert v0.get_length() == pytest.approx(23.52)
    assert v0.get_axle_weight(0) == pytest.approx(55.7115)
    assert v0.get_axle_weight(8) == pytest.approx(27.7560)
    assert v0.get_axle_spacing(0) == pytest.approx(3.58154)
    assert v0.get_axle_spacing(7) == pytest.approx(1.21846)
    assert v0.get_axle_spacing(8) == pytest.approx(0.0)

    v1 = vehicles[1]
    assert v1.get_no_axles() == 2
    assert v1.get_local_lane() == 2
    assert v1.get_velocity() == pytest.approx(29.9707)
    assert v1.get_gvw() == pytest.approx(16.4802)
    assert v1.get_axle_spacing(0) == pytest.approx(2.63415)
    assert v1.get_axle_spacing(1) == pytest.approx(0.0)
