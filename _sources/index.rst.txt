.. PyBTLS documentation master file, created by
   sphinx-quickstart on Tue Apr 16 14:00:44 2013.

.. figure:: images/logo/PyBTLS_logo.png
   :alt: PyBTLS logo
   :align: center
   :scale: 40

|Code style: black|
|License: GPLv3|
.. |version|
|Github issues|
|Github pull requests|
|PyPI|
..    |GitHub Workflow Deploy|
..    |GitHub Workflow Build|
..    |GitHub Workflow Status|
..    |GitHub commit activity|
..    |GitHub last commit|
..    |Contributor Covenant|
|codecov|

.. |Code style: black| image:: https://img.shields.io/badge/code%20style-black-000000.svg 
   :target: https://github.com/psf/black

.. |License: GPLv3| image:: https://img.shields.io/badge/License-GPLv3-yellow 
   :target: https://opensource.org/license/gpl-3-0/

.. 
.. |version| image:: https://img.shields.io/github/downloads/ccaprani/btls/total?label=version

.. |GitHub issues| image:: https://img.shields.io/github/issues/ccaprani/btls?logoColor=yellowgreen

.. |GitHub pull requests| image:: https://img.shields.io/github/issues-pr/ccaprani/btls?color=yellowgreen

.. |PyPI| image:: https://img.shields.io/pypi/v/pybtls

.. |GitHub Workflow Deploy| image:: https://img.shields.io/github/workflow/status/ccaprani/btls/Build%20and%20deploy

.. |GitHub Workflow Build| image:: https://img.shields.io/github/workflow/status/ccaprani/btls/Deploy%20to%20GitHub%20Pages?label=gh%20page%20build

.. |GitHub Workflow Status| image:: https://img.shields.io/github/workflow/status/ccaprani/btls/Tests?label=Tests

.. |GitHub commit activity| image:: https://img.shields.io/github/commit-activity/m/ccaprani/btls

.. |GitHub last commit| image:: https://img.shields.io/github/last-commit/ccaprani/btls?color=ff69b4

.. |Contributor Covenant| image:: https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg 
   :target: code_of_conduct.md

.. |codecov| image:: https://codecov.io/gh/ccaprani/btls/branch/main/graph/badge.svg?token=dUTOmPBnyP 
   :target: https://codecov.io/gh/ccaprani/btls

Welcome to PyBTLS's documentation!
==================================

:Date: June 2024
:Authors: The PyBTLS Developers (Colin Caprani, Ziyi Zhou, Akbar Rizqiansyah)
:Web site: http://github.com/ccaprani/btls
:Copyright: This document has been placed in the public domain.
:License: PyBTLS is released under the GNU General Public Licence v3.0.
:Version: 0.3.7

PyBTLS is a Python wrapper for the C++ based Bridge Traffic Load Simulation (BTLS) program, designed for traffic simulations on short-to-medium length bridges where lane-changing is negligible. It calculates load effects using influence lines or surfaces, with several built-in options and user-defined possibilities. Features include generating new traffic from historical data, efficient simulation with C++ backend and Python multiprocessing, and well-organized output in pandas DataFrame. 

.. note::

   If you have any problems, found bugs in the code or have feature request
   comments or questions, please raise an issue in the Github 
.. _`issue tracker`: http://github.com/ccaprani/btls/issues .


Contents:

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   install
   tutorial
   api
   theory
   references
   developer

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
