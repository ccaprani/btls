"""
setup example see https://github.com/pybind/python_example/blob/master/setup.py
"""

import platform
from glob import glob
from setuptools import setup
from setuptools.command.build_ext import build_ext as _build_ext
from pybind11.setup_helpers import Pybind11Extension, ParallelCompile, naive_recompile

# Install parallel compilation
ParallelCompile(
    "NPY_NUM_BUILD_JOBS", needs_recompile=naive_recompile, default=4
).install()

# Custom build_ext command to handle the debug flag
"""
Usage:
For a Regular Build or Install:

Run the setup script without any additional arguments:

    pip install .

For a Debug Build or Install:

    pip install . --config-settings="build_ext.debug_build=true"
    
To clean for recompilation:

    python setup.py clean --all

"""


class build_ext(_build_ext):
    def finalize_options(self):
        super().finalize_options()
        if self.distribution.get_option_dict("build_ext").get("debug_build"):
            self.debug_build = self.distribution.get_option_dict("build_ext")[
                "debug_build"
            ][1]
        else:
            self.debug_build = None

    def build_extensions(self):
        cpp_extra_compile_args = ["-fPIC"]

        # Check for macOS and add specific flags
        if platform.system() == "Darwin":
            cpp_extra_compile_args.extend(
                ["-stdlib=libc++", "-mmacosx-version-min=10.15", "-std=c++17"]
            )

        # Check for Windows and add specific flags
        elif platform.system() == "Windows":
            cpp_extra_compile_args.append("/std:c++17")

        # Check for other Unix-like systems and add specific flags
        else:
            cpp_extra_compile_args.append("-std=c++17")

        # Add debug flag if needed
        if self.debug_build:
            cpp_extra_compile_args.append("-g")

        # Apply the compile args to each extension
        for ext in self.extensions:
            ext.extra_compile_args = cpp_extra_compile_args

        super().build_extensions()


source_files = sorted(glob("./cpp/src/*.cpp", recursive=True))
try:
    source_files.remove("./cpp/src/main.cpp")  # for MacOS and Linux
except ValueError:
    source_files.remove("./cpp/src\\main.cpp")  # for Windows


ext_modules = [
    Pybind11Extension(
        name="pybtls.lib.libbtls",
        sources=source_files,
        include_dirs=["cpp/include"],
        cxx_std=17,
        language="c++",
        define_macros=[("PyBTLS", "")],  # To unlock some C++ methods.
    ),
]

setup(
    name="pybtls",
    cmdclass={"build_ext": build_ext},
    ext_modules=ext_modules,
)
