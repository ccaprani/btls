from pybtls import Vehicle
from pathlib import Path
import pickle


def check_vehicle(veh_str, veh_str_format):
    veh = Vehicle(0)
    veh._create(veh_str, veh_str_format)

    try:
        for i in range(veh.get_no_axles()):
            veh.get_axle_spacing(i)
            veh.get_axle_weight(i)
            veh.get_axle_width(i)
        veh.get_gvw()
        veh.get_length()
        veh.get_velocity()
        veh.get_time()
        veh.get_local_lane()
        veh.get_trans()
        veh.get_direction()
    except Exception as e:
        assert False, f"Error: {e}"

    with open(Path(__file__).parent / "temp/vehicle.pkl", "wb") as f:
        pickle.dump(veh, f)

    with open(Path(__file__).parent / "temp/vehicle.pkl", "rb") as f:
        veh_restored = pickle.load(f)

    assert veh == veh_restored


def test_MON():
    veh_str_format = 4  # MON format
    veh_str_list = [
        "     1001 1 12010 0 0 1613 2 0 16421 90 5620101800 4784 562011638    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0",
        "     1001 1 12010 0 0 4677 2 0  4943 78 5724201800 1492 5724 3451    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0",
        "     1001 1 12010 0 0 8270 4 0 46214 8610118101800 7121 327816523 473211285 210711285    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0",
        "     1001 1 12010 0 013771 5 0 43619 8210926101800 7134 286813860 5663 7541 1008 7541 1387 7541    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0",
        "     1001 1 12010 0 016681 5 0 33078 9210943201800 6265 298310492 5658 5440 1250 5440 1052 5440    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0",
    ]
    for veh_str in veh_str_list:
        check_vehicle(veh_str, veh_str_format)


def test_CASTOR():
    veh_str_format = 1  # CASTOR format
    veh_str_list = [
        "1001 1 1 0 0 0 885251 221 51211 18 7251149 0  0 0  0 0  0 0  0 0  0 0  0 0  0",
        "1001 1 1 0 0 01378230 318114511 18 553211157 5113 5113 51 0  0 0  0 0  0 0  0",
        "1001 1 1 0 0 01973199 310112422 18 563410966 7313 73 0  0 0  0 0  0 0  0 0  0",
        "1001 1 1 0 0 02134249 389109511 18 603310756 7412 74 8 74 0  0 0  0 0  0 0  0",
        "1001 1 1 0 0 02464217 346114422 18 603310466 9115 91 0  0 0  0 0  0 0  0 0  0",
    ]
    for veh_str in veh_str_list:
        check_vehicle(veh_str, veh_str_format)
