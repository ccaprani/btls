Installation
============

PyBTLS is known to run on Mac OS X, Linux and Windows. 
For the local build, it requires a C++ compiler with C++17 support, 
such as Clang (MacOS), GCC (Linux) and MSVC (Windows). 

Required Dependencies
---------------------
- Python 3.9 or later
- matplotlib
- numpy
- pandas
- scipy

Installation
------------
The easiest way to install `PyBTLS` is to use the python package index: ::

    pip install pybtls

For users wishing to develop: ::

    git clone https://github.com/ccaprani/btls.git
    cd pybtls
    pip install -e .

For users wishing to install it as BTLS (a C++ terminal program), the first is to ensure CMake and GCC are installed. 
Then: ::

    git clone https://github.com/ccaprani/btls.git
    cd pybtls
    mkdir ./build
    cd ./build
    cmake ../
    make -j 4

The executable file will be in the `bin` folder.

For contributions, first fork the repo and clone from your fork.
`Here <https://www.dataschool.io/how-to-contribute-on-github/>`_ is a good guide on this workflow.

Tests
-----
`PyBTLS` comes with ``pytest`` functions to verify the correct functioning of the package. 
Users can test this using: ::

    python -m pytest

from the root directory of the package.

Bugs and feature requests
-------------------------
Report problems with the installation, bugs in the code or feature request at the `issue tracker <http://github.com/ccaprani/btls/issues>_`

