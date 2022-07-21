# This is the example of a standard BTLS analysis. 
import BtlsPy


btls = BtlsPy.BTLS("BTLSin.txt")

pVC = btls.get_vehicle_classification()

vBridges = btls.get_bridges()

vLanes = btls.get_lanes(pVC)

btls.do_simulation(pVC, vBridges, vLanes, btls.StartTime, btls.EndTime)
