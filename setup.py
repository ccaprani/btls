from glob import glob
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, ParallelCompile, naive_recompile

__version__ = "0.2.0"

# `N` is to set the bumer of threads
# `naive_recompile` makes it recompile only if the source file changes. It does not check header files!
ParallelCompile("NPY_NUM_BUILD_JOBS", needs_recompile=naive_recompile, default=4).install()

# could only be relative paths, otherwise the `build` command would fail if you use a MANIFEST.in to distribute your package
# only source files (.cpp, .c, .cc) are needed
source_files = sorted(glob("./c_src/src/*.cpp", recursive=True))
source_files.remove("./c_src/src/main.cpp")

# If any libraries are used, e.g. libabc.so
include_dirs = ["./c_src/include"]
# library_dirs = ["LINK_DIR"]
# (optional) if the library is not in the dir like `/usr/lib/`
# either to add its dir to `runtime_library_dirs` or to the env variable "LD_LIBRARY_PATH"
# MUST be absolute path
# runtime_library_dirs = [os.path.abspath("LINK_DIR")]
# libraries = ["abc"]

ext_modules = [
    Pybind11Extension(
        name="BtlsPy.c_module._core", # depends on the structure of your package
        sources=source_files,
        # Example: passing in the version to the compiled code
        # define_macros=[("VERSION_INFO", __version__)],
        include_dirs=include_dirs,
        # library_dirs=library_dirs,
        # runtime_library_dirs=runtime_library_dirs,
        # libraries=libraries,
        cxx_std=14,
        language="c++"
    ),
]


setup(
    version = __version__,
    ext_modules=ext_modules,
)
