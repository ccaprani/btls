import pybtls
import pytest


def test_CASTOR_format():
    veh_str = (
        "1001 1 1 0 0 0 885251 221 51211 18 7251149 0  0 0  0 0  0 0  0 0  0 0  0 0  0"
    )
    veh_info = [
        1001,
        8 + 85 / 100,
        251 / 10,
        221 / 10 * 9.81,
        51 / 10,
        2,
        1,
        1,
        18 / 10,
        72 / 10 * 9.81,
        51 / 10,
        149 / 10 * 9.81,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    ]
    veh_info.pop(0)
    WIM_format_id = 1

    vehicle = pybtls.Vehicle(no_axles=2)
    vehicle._create(veh_str, WIM_format_id)
    vehicle_info = [
        vehicle.get_time(),
        vehicle.get_velocity(),
        vehicle.get_gvw(),
        vehicle.get_length(),
        vehicle.get_no_axles(),
        vehicle.get_direction(),
        vehicle.get_local_lane(),
        vehicle.get_trans(),
    ]
    temp_list = []
    for i in range(9):
        temp_list.append(vehicle.get_axle_weight(i))
        temp_list.append(vehicle.get_axle_spacing(i))
    vehicle_info.extend(temp_list)
    vehicle_info.pop(-1)

    assert veh_info == pytest.approx(vehicle_info, abs=0.001)


def test_MON_format():
    veh_str = "     1001 1 12010 0 0 1613 2 1 16421 90 5620101800 4784 562011638    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0"
    veh_info = [
        1001,
        1613 / 1000,
        2,
        1,
        16421 / 1000 * 9.81,
        90 / 3.6,
        5620 / 1000,
        1,
        0 + 1,
        1800 / 1000,
        4784 / 1000 * 9.81,
        5620 / 1000,
        11638 / 1000 * 9.81,
        0,
    ] + [0] * 2 * 13
    veh_info.pop(0)
    WIM_format_id = 4

    vehicle = pybtls.Vehicle(no_axles=2)
    vehicle._create(veh_str, WIM_format_id)
    vehicle_info = [
        vehicle.get_time(),
        vehicle.get_no_axles(),
        1,
        vehicle.get_gvw(),
        vehicle.get_velocity(),
        vehicle.get_length(),
        vehicle.get_local_lane(),
        vehicle.get_direction(),
        vehicle.get_trans(),
    ]
    temp_list = []
    for i in range(15):
        temp_list.append(vehicle.get_axle_weight(i))
        temp_list.append(vehicle.get_axle_spacing(i))
    vehicle_info.extend(temp_list)

    assert veh_info == pytest.approx(vehicle_info, abs=0.001)
