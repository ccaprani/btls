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
    assert isinstance(BtlsPy.FatigueCalculation(False,"BL_4.6_Fatigue.txt",1,"SN_4.6_Eff1.csv",2,False,1.6).fatigue_index,float)

    print("Tests were all passed!")


if __name__ == '__main__':
    test()