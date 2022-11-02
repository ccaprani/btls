import PyBTLS


def print_vehicle(veh):
    axs = []
    axw = []
    for i in range(veh.get_no_axles()):
        axs.append(veh.get_axle_spacing(i))
        axw.append(veh.get_axle_weight(i))

    print(f"No. axles = {veh.get_no_axles()}")
    print(f"Axle spacing = {axs}")
    print(f"Axle weight = {axw}")
    print(f"GVW = {veh.get_gvw()}")
    print(f"Speed = {veh.get_velocity()}")
    print(f"Length = {veh.get_length()}")

    print(f"Head = {veh.get_head()}")
    print(f"Time = {veh.get_time()}")
    print(f"Time string = {veh.get_time_str()}")
    print(f"Trans = {veh.get_trans()}")
    print(f"Dir = {veh.get_direction()}")


def print_test(veh_list, veh_format):
    for veh_str in veh_list:
        veh = PyBTLS.Vehicle()
        veh.create(veh_str, veh_format)
        print(80 * "*")
        print(veh_str)
        print(80 * "*")
        print_vehicle(veh)


def test_MON():
    veh_str_format = 4  # MON format
    veh_str_list = [
        "100001 1 12010 0 0 7508 4 0 29283 9410993101800 5826 3064 9429 6054 7014 1875 7014    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0",
        "100001 1 12010 0 0 9975 5 0 46628 8010986201800 8267 317914588 5463 7924 1007 7924 1338 7924    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0",
        "100001 1 12010 0 011090 2 0 20502 90 5399101800 6381 539914121    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0",
        "100001 1 12010 0 014019 5 0 27929 8310723201800 6616 3304 9799 5220 3838 1110 3838 1089 3838    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0",
        "100001 1 12010 0 016850 5 0 37715 9210035101800 6190 342012973 4394 6184 1032 6184 1189 6184    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0    0",
    ]

    print_test(veh_str_list, veh_str_format)


def test_CASTOR():
    veh_str_format = 1  # CASTOR format
    veh_str_list = [
        "1001 1 1 0 0 0 885251 221 51211 18 7251149 0  0 0  0 0  0 0  0 0  0 0  0 0  0",
        "1001 1 1 0 0 01378230 318114511 18 553211157 5113 5113 51 0  0 0  0 0  0 0  0",
        "1001 1 1 0 0 01973199 310112422 18 563410966 7313 73 0  0 0  0 0  0 0  0 0  0",
        "1001 1 1 0 0 02134249 389109511 18 603310756 7412 74 8 74 0  0 0  0 0  0 0  0",
        "1001 1 1 0 0 02464217 346114422 18 603310466 9115 91 0  0 0  0 0  0 0  0 0  0",
    ]
    print_test(veh_str_list, veh_str_format)
