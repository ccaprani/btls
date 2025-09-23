import pybtls as pb
import pytest


SEC_PER_MIN = 60
SEC_PER_HOUR = 60 * SEC_PER_MIN
SEC_PER_DAY = 24 * SEC_PER_HOUR
SEC_PER_MONTH = 25 * SEC_PER_DAY
SEC_PER_YEAR = 10 * SEC_PER_MONTH


def test_CASTOR_format():
    veh_str = (
        "8755 1 1 01052 740228 188175711 18 5743 3013 2754 2412 1741 1712 15 0  0 0  0"
    )
    veh_info = [
        8755,
        10*SEC_PER_HOUR + 52*SEC_PER_MIN + 7 + 40 / 100,
        228 / 10,
        188 / 10 * 9.81,
        175 / 10,
        7,
        1,
        1,
        18 / 10,
        57 / 10 * 9.81,
        43 / 10,
        30 / 10 * 9.81,
        13 / 10,
        27 / 10 * 9.81,
        54 / 10,
        24 / 10 * 9.81,
        12 / 10,
        17 / 10 * 9.81,
        41 / 10,
        17 / 10 * 9.81,
        12 / 10,
        15 / 10 * 9.81,
        0.0,
    ]
    veh_info.pop(0)
    WIM_format_id = 1

    vehicle = pb.Vehicle(no_axles=0)
    vehicle._create(veh_str, WIM_format_id)
    vehicle_info = [
        vehicle.get_time(),  # The moment of the day in seconds
        vehicle.get_velocity(),
        vehicle.get_gvw(),
        vehicle.get_length(),
        vehicle.get_no_axles(),
        vehicle.get_direction(),
        vehicle.get_local_lane(),
        vehicle.get_trans(),
    ]
    temp_list = []
    for i in range(vehicle.get_no_axles()):
        temp_list.append(vehicle.get_axle_weight(i))
        temp_list.append(vehicle.get_axle_spacing(i))
    vehicle_info.extend(temp_list)

    assert vehicle_info == pytest.approx(veh_info, abs=0.001)


def test_BeDIT_format():
    veh_str = (
        "4883 1 1 0 0 02195223 713212 901 18 52 33112 12 98 65 72 12 76 12 74 53 75 12 76 12 78  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0"
    )
    veh_info = [
        4883,
        21 + 95/100,
        223 / 10,
        713 / 10 * 9.81,
        212 / 10,
        9,
        0 + 1,
        1,
        18 / 10,
        52 / 10 * 9.81,
        33 / 10,
        112 / 10 * 9.81,
        12 / 10,
        98 / 10 * 9.81,
        65 / 10,
        72 / 10 * 9.81,
        12 / 10,
        76 / 10 * 9.81,
        12 / 10,
        74 / 10 * 9.81,
        53 / 10,
        75 / 10 * 9.81,
        12 / 10,
        76 / 10 * 9.81,
        12 / 10,
        78 / 10 * 9.81,
        0.0,
    ]
    veh_info.pop(0)
    WIM_format_id = 2

    vehicle = pb.Vehicle(no_axles=2)
    vehicle._create(veh_str, WIM_format_id)
    vehicle_info = [
        vehicle.get_time(),  # Just the seconds
        vehicle.get_velocity(),
        vehicle.get_gvw(),
        vehicle.get_length(),
        vehicle.get_no_axles(),
        vehicle.get_direction(),
        vehicle.get_local_lane(),
        vehicle.get_trans(),
    ]
    temp_list = []
    for i in range(vehicle.get_no_axles()):
        temp_list.append(vehicle.get_axle_weight(i))
        temp_list.append(vehicle.get_axle_spacing(i))
    vehicle_info.extend(temp_list)

    assert vehicle_info == pytest.approx(veh_info, abs=0.001)


def test_DITIS_format():
    veh_str = (
        "3828 1 1   0 0575117226 269215 911180 47198 33 40198 13 38198 57 26198 12 26198 12 29198 61 19198 14 22198 13 22198  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0"
    )
    veh_info = [
        3828,
        0*SEC_PER_HOUR + 57*SEC_PER_MIN + 51 + 17/100,
        226 / 10,
        269 / 10 * 9.81, 
        215 / 10,
        9,
        1,
        1,
        180 / 100,
        47 / 10 * 9.81,
        198 / 100,
        33/10,
        40 / 10 * 9.81,
        198 / 100,
        13 / 10,
        38 / 10 * 9.81,
        198 / 100,
        57 / 10,
        26 / 10 * 9.81,
        198 / 100,
        12 / 10,
        26 / 10 * 9.81,
        198 / 100,
        12 / 10,
        29 / 10 * 9.81,
        198 / 100,
        61 / 10,
        19 / 10 * 9.81,
        198 / 100,
        14 / 10,
        22 / 10 * 9.81,
        198 / 100,
        13 / 10,
        22 / 10 * 9.81,
        198 / 100,
        0.0,
    ]
    veh_info.pop(0)
    WIM_format_id = 3

    vehicle = pb.Vehicle(no_axles=2)
    vehicle._create(veh_str, WIM_format_id)
    vehicle_info = [
        vehicle.get_time(),  # The moment of the day in seconds
        vehicle.get_velocity(),
        vehicle.get_gvw(),
        vehicle.get_length(),
        vehicle.get_no_axles(),
        vehicle.get_direction(),
        vehicle.get_local_lane(),
        vehicle.get_trans(),
    ]
    temp_list = []
    for i in range(vehicle.get_no_axles()):
        temp_list.append(vehicle.get_axle_weight(i))
        temp_list.append(vehicle.get_axle_width(i))
        temp_list.append(vehicle.get_axle_spacing(i))
    vehicle_info.extend(temp_list)

    assert vehicle_info == pytest.approx(veh_info, abs=0.001)


def test_MON_format():
    veh_str = "   550586 1 12010 0 022130 9 4 33923 8222477101800 6096 3310 4683 1300 4563 5915 3068 1149 3347 1155 3417 7058 2927 1294 2864 1295 2958    0"
    veh_info = [
        550586,
        22130 / 1000,
        9,
        4,
        33923 / 1000 * 9.81,
        82 / 3.6,
        22477 / 1000,
        1,
        0 + 1,
        1800 / 1000,
        6096 / 1000 * 9.81,
        3310 / 1000,
        4683 / 1000 * 9.81,
        1300 / 1000,
        4563 / 1000 * 9.81,
        5915 / 1000,
        3068 / 1000 * 9.81,
        1149 / 1000,
        3347 / 1000 * 9.81,
        1155 / 1000,
        3417 / 1000 * 9.81,
        7058 / 1000,
        2927 / 1000 * 9.81,
        1294 / 1000,
        2864 / 1000 * 9.81,
        1295 / 1000,
        2958 / 1000 * 9.81,
        0.0,
    ]
    veh_info.pop(0)
    WIM_format_id = 4

    vehicle = pb.Vehicle(no_axles=2)
    vehicle._create(veh_str, WIM_format_id)
    vehicle_info = [
        vehicle.get_time(),  # Just the seconds
        vehicle.get_no_axles(),
        4,  # We do not provide the get_no_axle_groups() function since other formats do not have this info
        vehicle.get_gvw(),
        vehicle.get_velocity(),
        vehicle.get_length(),
        vehicle.get_local_lane(),
        vehicle.get_direction(),
        vehicle.get_trans(),
    ]
    temp_list = []
    for i in range(vehicle.get_no_axles()):
        temp_list.append(vehicle.get_axle_weight(i))
        temp_list.append(vehicle.get_axle_spacing(i))
    vehicle_info.extend(temp_list)

    assert vehicle_info == pytest.approx(veh_info, abs=0.001)
