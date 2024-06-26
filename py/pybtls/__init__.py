"""
Bridge Traffic Load Simulation with Python. 
"""
import os
import sys

__version__ = "0.3.7"  # Should also manually update in pyproject.toml

# Add the directory containing the shared library to sys.path
lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "lib"))
if lib_path not in sys.path:
    sys.path.insert(0, lib_path)

import pybtls.lib as lib
import pybtls.bridge as bridge
import pybtls.traffic as traffic
import pybtls.output as output
import pybtls.simulation as simulation
import pybtls.garage as garage
import pybtls.utils as utils

# import pybtls.post_processing as post_processing

from .lib import (
    Vehicle,
    MultiModalNormal,
    Distribution,
    run,
    get_info,
)
from .bridge import InfluenceLine, InfluenceSurface, Bridge
from .traffic import (
    LaneFlowComposition,
    VehicleGenGarage,
    VehicleGenGrave,
    VehicleGenNominal,
    HeadwayGenNHM,
    HeadwayGenConstant,
    HeadwayGenCongested,
    HeadwayGenFreeflow,
    TrafficGenerator,
    TrafficLoader,
)
from .output import OutputConfig, Read, Plot
from .simulation import Simulation
from .utils import save_output, load_output
