"""
Garage (vehicle) file reading and writing.
"""

from pathlib import Path

import pytest

import pybtls as pb


def test_garage_round_trip(tmp_path):
    # Vehicle files must round-trip: read -> write -> read preserves every
    # vehicle's fields (the values are already quantised by the format).
    src = Path(__file__).parent / "test_data/garage.txt"
    vehicles = pb.garage.read_garage_file(garage_path=src, garage_format=4)

    rewritten = tmp_path / "rewritten.txt"
    pb.garage.write_garage_file(vehicles, rewritten, 4)
    again = pb.garage.read_garage_file(garage_path=rewritten, garage_format=4)

    assert len(vehicles) == len(again)
    for v1, v2 in zip(vehicles, again):
        assert v1.get_time() == v2.get_time()
        assert v1.get_gvw() == pytest.approx(v2.get_gvw(), rel=1e-9)
        assert v1.get_no_axles() == v2.get_no_axles()
        assert v1.get_velocity() == pytest.approx(v2.get_velocity(), rel=1e-9)


def test_write_garage_file_accepts_str_path(tmp_path):
    vehicle = pb.Vehicle(no_axles=2)
    vehicle.set_axle_weights([80.0, 80.0])
    vehicle.set_axle_spacings([4.0, 0.0])
    vehicle.set_axle_widths([2.0, 2.0])

    out_path = str(tmp_path / "garage_str_path.txt")
    pb.garage.write_garage_file([vehicle], out_path, 4)

    assert (tmp_path / "garage_str_path.txt").exists()
