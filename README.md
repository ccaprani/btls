# PyBTLS - Python Bridge Traffic Load Simulation

![PyBTLS logo](https://raw.githubusercontent.com/ccaprani/btls/main/docs/source/images/PyBTLS_logo.png)

`PyBTLS` is a python wrapper of the C++ based program bridge traffic load simulation (BTLS). 
It is tailored for traffic simulations on short-to-medium length bridges, where vehicles' lane-changing behaviors are considered negligible.

`PyBTLS` uses influence lines or surfaces to calculate the load effects on bridges. 
It integrates several built-in influence lines (FIGURE), and more influence lines or surfaces can be defined by users. 

The program features:

- Pre-processing of WIM data (e.g., classifying vehicles, filtering traffic, etc.); 
- Conducting traffic simulation (e.g., load effects calculation, traffic generation, etc.);
- Post-processing of the simulation outcomes (e.g., block-maximum analysis, peak-over-threshold analysis, rainflow analysis, etc.). 

---
> See the [**documents**](https://ccaprani.github.io/btls/) or [**manual**](https://raw.githubusercontent.com/ccaprani/btls/main/Manual) for more details.
---

To **install** this project online as a Python package: 

1. Run `pip install PyBTLS`. 


To **build** this project locally as a program (requires CMake and g++): 

1. Download and open the PyBTLS folder in a terminal; 

2. Run `mkdir ./build`; 

3. Run `cd ./build`; 

4. Run `cmake`;

5. Run `make -j 4`. 

The executable file will be in the `./bin` folder.


To **install** this project locally as a Python package: 

1. Download and open the PyBTLS folder in a terminal;  

2. Run `pip install .`. 


Finish! You can import PyBTLS in Python now! See the [**here**](https://raw.githubusercontent.com/ccaprani/btls/main/docs/source/notebooks) for examples. 

