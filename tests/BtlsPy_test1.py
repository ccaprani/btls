# This is the example of a standard BTLS analysis. 
import BtlsPy


def test():
    btls = BtlsPy.BTLS()
    ## Test run entirely
    assert btls.run("BTLSin.txt") == 1

    ## Test run separately
    # xxx_config = BtlsPy.CConfigData.xxx_Config()
    # btls.set_xxx_config(xxx_config)
    # ...
    # btls.set_program_mode(1)
    pVC = btls.get_vehicle_classification()
    vBridges = btls.get_bridges()
    vLanes = btls.get_lanes(pVC)
    assert btls.do_simulation(pVC,vBridges,vLanes,btls.StartTime,btls.EndTime) == 1

    ## Test RN-curve fatigue method
    fatigue_test = BtlsPy.FatigueCalculation()
    assert isinstance(fatigue_test.sim_and_analyse(["SN_test.txt","SN_test.txt","SN_test.txt","SN_test.txt","SN_test.txt"],False,[[1,1],[1,1],[1,1],[1,1],[1,1]]),list)
    assert isinstance(fatigue_test.read_and_analyse("TH_4.6.txt","SN_test.txt",False,[1.8],[1]),list)

    print("Tests were all passed!")


if __name__ == '__main__':
    test()