# BTLS Parallel Branch
By Akbar

Added modules to:
1. Define bridge and traffic
2. Perform analysis in single or parallel
3. Extract BTLS output neatly into Pandas DataFrame, ready for analysis

These modules eliminates the need for the user to interact with the .txt files. All can be done in Python using OOP syntaxes.
For an example, see in the folder tests/Example analysis of block maxima.ipynb

To Do:
- Convert grave_traffic files into one that is editable using Pandas DataFrame syntaxing
- Importing of all simulation outputs. Only several are supported right now. This is only a matter of coding the txt parser.
- Saving and loading functionalities. Pickle module somewhat works for now, but it doesn't save some attributes/properties. See the example file.

Please reach out if you have any comments, questions, suggestions, etc.


# BTLS

**Bridge Traffic Load Simulation by Colin Caprani (Modified by Ziyi Zhou).**

---
> See the **manual** in the `./Manual` folder for details.

> To **build** this project as a program: 
    1. Open the PyBTLS folder in a ternimal; 
    2. Run `mkdir ./build`; 
    3. Run `cd ./build`; 
    4. Run `cmake`;
    5. Run `make -j 4`. 
    The executable file will be in the `./bin` folder.

> To **build** this project as a Python extension: 
    1. Open the PyBTLS folder in a ternimal;  
    2. Run `pip install .`; 
    Finish! You can import PyBTLS in Python now! See the `./tests` folder for example. 
