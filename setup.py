from glob import glob
from distutils.core import setup

__version__ = "2.0.0"

try:
    from pybind11.setup_helpers import Pybind11Extension, ParallelCompile, naive_recompile

    # `N` is to set the bumer of threads
    # `naive_recompile` makes it recompile only if the source file changes. It does not check header files!
    ParallelCompile("NPY_NUM_BUILD_JOBS", needs_recompile=naive_recompile, default=4).install()

    # could only be relative paths, otherwise the `build` command would fail if you use a MANIFEST.in to distribute your package
    # only source files (.cpp, .c, .cc) are needed
    source_files = sorted(glob("src/*.cpp"))

    # If any libraries are used, e.g. libabc.so
    include_dirs = ["./include"]
    # library_dirs = ["LINK_DIR"]
    # (optional) if the library is not in the dir like `/usr/lib/`
    # either to add its dir to `runtime_library_dirs` or to the env variable "LD_LIBRARY_PATH"
    # MUST be absolute path
    # runtime_library_dirs = [os.path.abspath("LINK_DIR")]
    # libraries = ["abc"]

    ext_modules = [
        Pybind11Extension(
            "BtlsPy", # depends on the structure of your package
            source_files,
            # Example: passing in the version to the compiled code
            define_macros=[('VERSION_INFO', __version__)],
            include_dirs=include_dirs,
            # library_dirs=library_dirs,
            # runtime_library_dirs=runtime_library_dirs,
            # libraries=libraries,
            cxx_std=14,
            language='c++'
        ),
    ]
except ImportError:
    pass

setup(
    name='BtlsPy',  # used by `pip install`
    version=__version__,
    author="Colin Caparani & Ziyi Zhou",
    author_email="",
    url="",
    description="A package for bridge (short-to-mid span) traffic loading simulation.",
    long_description="",
    ext_modules=ext_modules,
    # packages=['package'], # the directory would be installed to site-packages
    setup_requires=["pybind11"],
    install_requires=["pybind11"],
    python_requires='>=3.6',
    include_package_data=True,
    zip_safe=False,
)