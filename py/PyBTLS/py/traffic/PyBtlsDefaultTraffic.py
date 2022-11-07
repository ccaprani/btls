"""
 NOTE:  OLD CODES, BUT STILL REQUIRED AS REPLACEMENT IS STILL BEING WORKED ON
"""

import numpy as np
import PyBTLS.py.traffic.PyBtlsTraffic as PyBtlsTraffic


auxerre_axle_spacing = PyBtlsTraffic.AxleSpacing()
auxerre_axle_spacing.load_default("Auxerre/ASall.csv")
auxerre_track_width = PyBtlsTraffic.AxleTrackWidth()
auxerre_track_width.load_default("Auxerre/ATW.csv")
auxerre_axle_weight_23 = PyBtlsTraffic.AxleWeight23()
auxerre_axle_weight_23.load_default("Auxerre/AW2&3.csv")
auxerre_axle_weight_45 = PyBtlsTraffic.AxleWeight45()
auxerre_axle_weight_45.load_default("Auxerre/AW4&5.csv")
auxerre_axle_gvw = PyBtlsTraffic.GrossVehicleWeight()
auxerre_axle_gvw.load_default("Auxerre/GVWpdf.csv")
auxerre_axle_headway = PyBtlsTraffic.Headway()
auxerre_axle_headway.load_default("Auxerre/NHM.csv")
auxerre_axle_flowrate = PyBtlsTraffic.FlowRate()
auxerre_axle_flowrate.load_default("Auxerre/FlowR.csv")
auxerre_axle_class_pct = PyBtlsTraffic.ClassPercentage()
auxerre_axle_class_pct.load_default("Auxerre/CLASS%.csv")

Auxerre = PyBtlsTraffic.BtlsTraffic(
    axle_spacing = auxerre_axle_spacing,
    axle_track_width = auxerre_track_width,
    axle_weight_23 = auxerre_axle_weight_23,
    axle_weight_45 = auxerre_axle_weight_45,
    gross_vehicle_weight = auxerre_axle_gvw,
    headway = auxerre_axle_headway,
    flow_rate = auxerre_axle_flowrate,
    class_percentage = auxerre_axle_class_pct
)