# This is the example of a standard BTLS analysis. 
import BtlsPy


btls = BtlsPy.BTLS("BTLSin.txt")

pVC = btls.get_vehicle_classification()

vBridges = btls.prepare_bridge()

if btls.IF_GEN_TRAFFIC:
    vLanes = btls.get_generator_lanes(pVC)
if btls.IF_READ_FILE:
    vLanes = btls.get_traffic_file_lanes(pVC)

btls.do_simulation(pVC, vBridges, vLanes, btls.StartTime, btls.EndTime)
