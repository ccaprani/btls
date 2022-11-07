This is just some general ideas on what a user could do with the python interface.
Do a brain dump and then we can categorize the ideas, and see what's feasible, and what changes to the C++ and/or python code is needed.

A user should be able to...

- [ ] create a vehicle in python & pass to BTLS for running over bridges/ILs
- [ ] read a vehicle file using BTLS into python & maybe do some statistics of the vehicle file
- [ ] generate vehicles in BTLS and pass to python, without needing to write to file
- [ ] use BTLS to get the classification of a vehicle object from python
- [ ] use python to load & set vehicle garage for that Vehicle Model (would be inefficient, but flexible)
- [ ] use the BTLS Flow Models to generate arrival times that are passed to python only (no simulation)
- [ ] use the BTLS Vehicle Models to generate vehicles without a flow model & pass to python (no simulation)
- [ ] obtain influence line from BTLS into python for plotting
- [x] use the BTLS distribution objects to generate random variables

Inputs:
- [ ] All inputs should be able to be set in pure python with no need for files (e.g. avoid BTLSin, ILs, Bridges, LaneFlow, etc)
- [ ] create bridges/ILs in python & pass to BTLS sidestepping the need to use files for these
- [ ] set the simulation parameters in python (e.g. mode such as Gen & Sim)
- [ ] set, read, edit any of the LaneFlow data

Outputs:

- [x] read BTLS outputs from files into python for processing/analysis
- [ ] plot things like critical loading events

Future idea: provide hooks back from the C++ code to python code, so that users can:
- [ ] provide a flow and/or vehicle model in python
- [ ] provide an IL function in python
- [ ] pass outputs back to pytyon without needing to write to file
