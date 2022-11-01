import platform
from glob import glob
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, ParallelCompile, naive_recompile

__version__ = "0.2.2"

# `N` is to set the bumer of threads
# `naive_recompile` makes it recompile only if the source file changes. It does not check header files!
ParallelCompile("NPY_NUM_BUILD_JOBS", needs_recompile=naive_recompile, default=4).install()

# could only be relative paths, otherwise the `build` command would fail if you use a MANIFEST.in to distribute your package
# only source files (.cpp, .c, .cc) are needed
source_files = sorted(glob("./cpp/src/*.cpp", recursive=True))
try:
    source_files.remove("./cpp/src/main.cpp")  # for MacOS and Linux
except ValueError:
    source_files.remove("./cpp/src\\main.cpp")  # for Windows

# If any libraries are used, e.g. libabc.so
include_dirs = ["./cpp/include"]
# library_dirs = ["LINK_DIR"]
# (optional) if the library is not in the dir like `/usr/lib/`
# either to add its dir to `runtime_library_dirs` or to the env variable "LD_LIBRARY_PATH"
# MUST be absolute path
# runtime_library_dirs = [os.path.abspath("LINK_DIR")]
# libraries = ["abc"]

# Extra parameters to the compiler
cpp_extra_compile_args = []
if platform.system() == "Darwin":
    # cpp_extra_compile_args.append('-stdlib=libc++')
    cpp_extra_compile_args.append('-mmacosx-version-min=10.15')
    # cpp_extra_link_args.append('-stdlib=libc++')
    # cpp_extra_link_args.append('-mmacosx-version-min=10.7')

# Set ext modules.
ext_modules = [
    Pybind11Extension(
        name="PyBTLS.cpp._core", # depends on the structure of your package
        extra_compile_args = cpp_extra_compile_args,
        sources=source_files,
        # Example: passing in the version to the compiled code
        # define_macros=[("VERSION_INFO", __version__)],
        include_dirs=include_dirs,
        # library_dirs=library_dirs,
        # runtime_library_dirs=runtime_library_dirs,
        # libraries=libraries,
        cxx_std=17,
        language="c++"
    ),
]


# Finish setup()
setup(
    version = __version__,
    ext_modules=ext_modules,
)
