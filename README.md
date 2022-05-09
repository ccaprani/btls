# BTLS
Bridge Traffic Load Simulation by Colin Caprani.

Modified by Ziyi Zhou. 

See the manual in the `Manual` folder for details.

The Windows Build folder contains a working set of files: just update the `BTLSin.txt` file to point to the correct location of the `Traffic` data folder, which is also supplied.

To build this project (for Linux, MacOS and Windows): 
1. Open the btls folder in ternimal; 
2. Run `mkdir ./build`; 
3. Run `cd ./build`; 
4. Run `cmake .. -DBuild=ON` for an executable file;
5. Run `make`. 

The execuatlbe file will be in btls/bin.

To create a Python package from this project (for Linux and MacOS): 
1. Run `mkdir ./build`; 
2. Run `cd ./build`; 
3. Run `cmake .. -DPy=ON` for a Python Package; 
4. Run `make`. 

To create a Python package from this project (for Windows): 
1. Download and install `Visual Studio 2019`;
2. Run `mkdir ./build`; 
3. Run `cd ./build`; 
4. Run `cmake .. -DPy=ON -G "Visual Studio 16 2019" -A x64` for a Python Package; 
5. Run `MSBuild ./BtlsPy.sln`. 

Finish! You can import BtlsPy in Python now!
