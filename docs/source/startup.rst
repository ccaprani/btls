***************
Getting Started
***************

This page helps you install PyBTLS and run your first simulation quickly. PyBTLS runs on macOS, Linux, and Windows. 

Prerequisites
-------------

- Python 3.10 or later with ``pip`` available.
- On Linux, the prebuilt wheel is x86_64 and needs glibc 2.27 or newer (Ubuntu 18.04+, Debian 10+, RHEL/Rocky/AlmaLinux 8+, Amazon Linux 2023). Check yours with ``ldd --version``. Older systems and other architectures install from the source distribution instead, so the source-install prerequisites below apply to them as well.
- For source installs: a C++17 compiler whose ``<filesystem>`` needs no separate library (GCC 9 or newer, Clang 9 or newer, MSVC 2017 15.7 or newer), ``cmake`` (>=3.25 is recommended), and ``ninja``.
- Core Python dependencies: ``matplotlib``, ``numpy``, ``pandas``, ``scipy`` (installed automatically via ``pip``).

Install from PyPI (recommended)
-------------------------------

1. (Optional) create and activate a virtual environment. ::

      python -m venv .venv && source .venv/bin/activate   # Linux/macOS
      python -m venv .venv && .venv\Scripts\activate     # Windows

2. Install PyBTLS. ::

      pip install pybtls

3. Verify the installation. In your Python interpreter, run: ::

      import pybtls as pb
      print(f"PyBTLS version: {pb.__version__}")

Install from source
-------------------

Use this workflow if you want editable installs to modify the code or contribute to development, or if your platform has no published wheel (see Prerequisites), in which case ``pip install pybtls`` builds this same source distribution for you. Local builds require GCC 9 or newer (Linux), Clang 9 or newer (macOS), or MSVC 2017 15.7 or newer (Windows): earlier compilers accept ``-std=c++17`` but need ``-lstdc++fs`` for ``std::filesystem``, which this build does not link.

1. Clone your fork (or the main repository). ::

      git clone https://github.com/ccaprani/btls.git
      cd btls

2. Create the development environment using your preferred environment manager. Here we use conda. If you don't have conda installed, see the `Miniconda install guide <https://docs.conda.io/en/latest/miniconda.html>`_. ::

      conda env create -f environment.yml
      conda activate pybtls-dev

3. Install in editable mode. ::

      pip install -e .

4. Optional: Install as the C++ terminal program BTLS (functions are limited). ::

      mkdir build && cd build
      cmake ../
      make -j4

Quick start
-----------

Save the snippet below as ``quickstart.py`` in an empty directory to avoid clashes with existing output folders. ::

    from pathlib import Path
    import pybtls as pb

    lfc = pb.LaneFlowComposition(lane_index = 1, lane_dir = 1)
    lfc.assign_lane_data(  # 24 hours per day by default
        hourly_truck_flow=[80] * 24,
        hourly_car_flow=[20] * 24,
        hourly_speed_mean=[80 / 3.6 * 10] * 24,  # in dm/s
        hourly_speed_std=[10 / 3.6 * 10] * 24,  # in dm/s
        hourly_truck_composition=[[23, 2.8, 31.7, 42.5]] * 24,
    )

    vehicle_gen = pb.VehicleGenGrave(traffic_site = "Auxerre")
    headway_gen = pb.HeadwayGenFreeflow()

    traffic_gen = pb.TrafficGenerator(no_lane = 1)
    traffic_gen.add_lane(
        vehicle_gen = vehicle_gen,
        headway_gen = headway_gen,
        lfc = lfc,
    )

    output_config = pb.OutputConfig()
    output_config.set_vehicle_file_output(write_vehicle_file = True)

    sim_task = pb.Simulation(output_dir = Path("./btls_quickstart"))  # Can specify a different output directory here.
    sim_task.add_sim(
        traffic = traffic_gen,
        no_day = 25,  # 25 working days
        output_config = output_config,
        tag = "demo",
    )
    sim_task.run()

Run it with ``python quickstart.py``; a new ``btls_quickstart/demo`` folder will contain the simulation output file (the generated traffic).

Run the test suite
------------------

From the repository root: ::

    python -m pytest

Getting help
------------

- Open an issue at https://github.com/ccaprani/btls/issues for installation or usage problems.
- For C++ build errors, check the compiler version and not only whether it accepts C++17: PyBTLS uses ``std::filesystem`` without linking ``stdc++fs``, so ``undefined reference to std::filesystem`` means the compiler is older than GCC 9, Clang 9 or MSVC 2017 15.7. Also check that ``cmake`` and ``ninja`` are on your ``PATH``.
- Each simulation tag creates a new output directory; remove or rename existing folders if a rerun fails because the directory already exists.

If you are not familiar with Python, you may find the `Python official tutorial <https://docs.python.org/3/tutorial/>`_ helpful. 
