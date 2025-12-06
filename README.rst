.. figure:: https://raw.githubusercontent.com/ccaprani/btls/main/docs/source/images/logo/PyBTLS_logo.png
   :alt: PyBTLS logo
   :align: center
   :width: 320px

**********************************************
PyBTLS - Python Bridge Traffic Load Simulation
**********************************************

*PyBTLS* is a Python wrapper of the C++ based terminal program BTLS (bridge traffic load simulation). 
It provides four main functionalities: 

* Generating new traffic; 
* Reading historical traffic; 
* Calculating load effect; 
* Processing the load effect data. 

*PyBTLS* has the following advantages: 

* Highly configurable traffic generation - The adjustable traffic statuses include flow type, traffic volume, composition, average speed, etc.; 
* Flexible traffic load calculation - Influence lines/surfaces are used; 
* Efficient simulation running - C++ backend and Python multiprocessing support; 
* Embedded load data processing - Block maximum, peak-over-threshold, rainflow; 
* Well-organized output - Result data are saved in structured folders and can be read as pandas DataFrame. 

However, please note that *PyBTLS* is not doing traffic microsimulation; 
Instead, it only applies a simplified vehicle-chase model to avoid vehicle overlapping on the bridge. 
Therefore, it should either be used for simulating traffic on short-to-medium length bridges, 
where the driver behaviours like lane-changing can be considered negligible due to the short crossing time; 
or for getting input traffic for more detailed microsimulation software. 

Installation
============

To install *PyBTLS*, please ensure that you have Python 3.10 or higher installed and are on a 64-bit system. 
Then, just do:

.. code-block:: bash

    pip install pybtls

If you want to build from the source or build as BTLS, see the `Installation Guide`_. 

.. _`Installation Guide`: https://ccaprani.github.io/btls/startup.html

After installation
==================
The following resources are available to help you get familiar with *PyBTLS*:

* The `Minimal Working Example`_ is a quick introduction to running a simple traffic simulation using *PyBTLS*. 
* The `Documentation`_ provides all the information needed to build *PyBTLS* and conduct different simulations supported by *PyBTLS*. More examples can be found in its `tutorials`_ section.
* The `Manual`_ is for the C++ terminal program BTLS, which includes more detailed information about the simulation mechanism. 

.. _`Minimal Working Example`: https://ccaprani.github.io/btls/notebooks/minimum_example.html
.. _`Documentation`: https://ccaprani.github.io/btls/
.. _`tutorials`: https://ccaprani.github.io/btls/tutorial.html
.. _`Manual`: https://github.com/ccaprani/btls/tree/main/Manual

Credits & Copyright
===================
Until 2025, 
PyBTLS is based on BTLS by Colin Caprani; 
Modified by Ziyi Zhou; 
Also contributed by Akbar Rizqiansyah. 

* Copyright (c) 2001-2022 Colin Caprani;
* Copyright (c) 2022-2024 Colin Caprani, Ziyi Zhou, Akbar Rizqiansyah;
* Copyright (c) 2025-* The PyBTLS developers (see contributors on GitHub). 

Main Reference
==============

O'Brien, E., Nowak, A., & Caprani, C. (Eds.). (2021). `Bridge traffic loading: From research to practice`_. CRC Press.

.. _`Bridge traffic loading: From research to practice`: https://books.google.com.au/books?hl=zh-CN&lr=&id=j9tKEAAAQBAJ&oi=fnd&pg=PP1&dq=Bridge+traffic+loading:+From+research+to+practice&ots=Pl6tyRIMb-&sig=NYrA_Docg2jJYymS-Z-w5x6lbRk#v=onepage&q=Bridge%20traffic%20loading%3A%20From%20research%20to%20practice&f=false
