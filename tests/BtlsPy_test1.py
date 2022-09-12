# This is the example of a standard BTLS analysis. 
import BtlsPy


def test(test_no:int):
    btls = BtlsPy.BTLS()

    if test_no == 1:
        ## Test run entirely
        assert btls.run("BTLSin.txt") == 1

    if test_no == 2:
        ## Test run separately
        settings = BtlsPy.Settings.gen_and_sim("./1-ABT6111Bridges.txt","./LaneFlowData_80.csv","../Traffic/Auxerre/",no_days=20,infline_file="./1-ABT6111ILS.txt")
        btls.set_road_config(settings["road_setting"])
        btls.set_gen_config(settings["gen_setting"])
        # btls.set_read_config(settings["read_setting"])
        # settings["traffic_setting"].xxx = xxx
        btls.set_traffic_config(settings["traffic_setting"])
        btls.set_sim_config(settings["sim_setting"])
        btls.set_output_config(settings["output_setting"])
        btls.set_program_mode(1)

        # pVC = btls.get_vehicle_classification()
        # vBridges = btls.get_bridges()
        # vLanes = btls.get_lanes(pVC)
        # assert btls.do_simulation(pVC,vBridges,vLanes,btls.StartTime,btls.EndTime,btls.get_rainflow_result()) == 1
        assert btls.run() == 1

    if test_no == 3:
        ## Test RN-curve fatigue method
        fatigue_test = BtlsPy.FatigueCalculation()
        assert isinstance(fatigue_test.sim_and_analyse(["SN_test.txt","SN_test.txt","SN_test.txt","SN_test.txt","SN_test.txt"],False,[[1,1],[1,1],[1,1],[1,1],[1,1]]),list)
        assert isinstance(fatigue_test.read_and_analyse("TH_4.6.txt","SN_test.txt",False,[1.8],[1]),list)

    print("Test passed!")


if __name__ == '__main__':
    test(int(input("Please select from 1, 2, 3: ")))