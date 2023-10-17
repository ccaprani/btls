Installation
============

Required Dependencies
---------------------
- Python 3.9 or later
- matplotlib
- numpy
- pandas
- scipy
- seaborn

Instructions
------------
The easiest way to install `PyBTLS` is to use the python package index: ::

    pip install PyBTLS

For users wishing to develop: ::

    git clone https://github.com/ccaprani/btls
    cd btls
    pip install -e .
    
For contributions, first fork the repo and clone from your fork. `Here <https://www.dataschool.io/how-to-contribute-on-github/>`_ is a good guide on this workflow.

Tests
-----
`PyBTLS` comes with ``pytest`` functions to verify the correct functioning of the package. 
Users can test this using: ::

    python -m pytest

from the root directory of the package.
