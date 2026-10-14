"""
The module that acts as the bridge between the C++ and Python code.\n
Refer to the API documentation for usage of the classes and functions.
"""

import os

from . import libbtls
from .BTLS import __version__, get_info, run, Vehicle, MultiModalNormal, Distribution

# The C++ random number generator is one process-wide state, seeded when the
# library loads. A forked child copies it, so it would draw the same numbers as
# its parent and siblings; reseed it from OS entropy, as Python's random module
# does. A spawned child loads the library afresh, and a seed() call in the child
# (as Simulation makes for a seeded run) still makes that run reproducible.
if hasattr(os, "register_at_fork"):  # POSIX only; Windows has no fork
    os.register_at_fork(
        after_in_child=lambda: libbtls.seed(int.from_bytes(os.urandom(8), "little"))
    )
