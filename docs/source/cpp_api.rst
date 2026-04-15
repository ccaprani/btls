C++ API Reference
=================

This page documents the C++ simulation core behind PyBTLS. The
:doc:`python_api` binds to these classes through pybind11 — users of the
Python API do not normally need to read this section. It is intended for
contributors extending the simulation core and researchers who want to
understand the numerical behaviour in detail.

.. note::

   C++ API documentation is being rolled out class by class on the
   ``docs-overhaul`` branch. The currently documented classes are listed
   below; additional classes will be added in subsequent commits.

Bridge and lanes
----------------

.. doxygenclass:: CBridge
   :project: pybtls
   :members:
   :protected-members:
   :private-members:

.. doxygenclass:: CBridgeLane
   :project: pybtls
   :members:
   :protected-members:
   :private-members:

Vehicles and axles
------------------

.. doxygenclass:: CVehicle
   :project: pybtls
   :members:
   :protected-members:
   :private-members:

.. doxygenclass:: CAxle
   :project: pybtls
   :members:
   :protected-members:
   :private-members:

Load effects
------------

.. doxygenclass:: CInfluenceLine
   :project: pybtls
   :members:
   :protected-members:
   :private-members:

.. doxygenclass:: CCalcEffect
   :project: pybtls
   :members:
   :protected-members:
   :private-members:

Events
------

.. doxygenclass:: CEventManager
   :project: pybtls
   :members:
   :protected-members:
   :private-members:

.. doxygenclass:: CEvent
   :project: pybtls
   :members:
   :protected-members:
   :private-members:

.. doxygenclass:: CEffect
   :project: pybtls
   :members:
   :protected-members:
   :private-members:

.. doxygenclass:: CEventBuffer
   :project: pybtls
   :members:
   :protected-members:
   :private-members:

Event output managers
---------------------

Both managers below derive from ``COutputManagerBase`` (see
``cpp/include/OutputManagerBase.h``), which factors out the shared file
handling. The shared base is documented inline in the header file and
is not rendered here as its own section — Breathe would emit inherited
symbols for each derived class, causing duplicate cross-references in
the same page.

.. doxygenclass:: CPOTManager
   :project: pybtls
   :members: Update

.. doxygenclass:: CBlockMaxManager
   :project: pybtls
   :members: Update

.. doxygenclass:: CBlockMaxEvent
   :project: pybtls
   :members:
   :protected-members:
   :private-members:

.. doxygenclass:: CEventStatistics
   :project: pybtls
   :members:
   :protected-members:
   :private-members:

Traffic streams (lanes)
-----------------------

Both lane classes below derive from ``CLane`` (see
``cpp/include/Lane.h``), which defines the ``GetNextVehicle`` contract
and caches the next-arrival-time bookkeeping. The abstract base is
documented inline in its header only — rendering it alongside its
derived classes causes duplicate inherited-symbol targets.

.. doxygenclass:: CLaneGenTraffic
   :project: pybtls
   :members: GetNextVehicle, setLaneData, initLane, GenNextArrival, GenNextTime, GenNextVehicle

.. doxygenclass:: CLaneFileTraffic
   :project: pybtls
   :members: GetNextVehicle, setLaneData, addVehicle, setFirstArrivalTime, GetNoVehicles

.. doxygenclass:: CVehicleTrafficFile
   :project: pybtls
   :members:
   :private-members:

.. doxygenclass:: CVehicleBuffer
   :project: pybtls
   :members:
   :private-members:

Vehicle generators
------------------

The vehicle generator hierarchy roots at ``CGenerator`` (see
``cpp/include/Generator.h``), which carries the random-number generator
and a handful of numerical helpers used by all derived samplers. The
intermediate ``CVehicleGenerator`` defines the sampling contract; the
three concrete classes below implement different modelling philosophies
(pool-based, parametric French-traffic Grave model, or a user-supplied
nominal). Both base classes are documented inline in their headers —
rendering them here alongside their derived classes causes duplicate
inherited-symbol targets. Members on the derived classes are listed
explicitly for the same reason.

.. doxygenclass:: CVehicleGenerator
   :project: pybtls
   :members: Generate, update, GenerateVehicle, GenerateCar, NextVehicleIsCar, GenVehClass, SetKernelGenerator

.. doxygenclass:: CVehicleGenGarage
   :project: pybtls
   :members: GenerateVehicle, GenVehClass, randomize

.. doxygenclass:: CVehicleGenGrave
   :project: pybtls
   :members: GenerateVehicle, GenVehClass, GenerateTruck23, GenerateTruck45, GenerateCommonProps

.. doxygenclass:: CVehicleGenNominal
   :project: pybtls
   :members: GenerateVehicle, GenVehClass, randomize

Axle distributions
------------------

.. doxygenclass:: CAxleSpacing
   :project: pybtls
   :members:
   :private-members:

.. doxygenclass:: CAxleWeight23
   :project: pybtls
   :members:
   :private-members:

.. doxygenclass:: CAxleWeight45
   :project: pybtls
   :members:
   :private-members:

Flow (headway / speed) generators
---------------------------------

The flow generator hierarchy roots at ``CFlowGenerator`` (also derived
from ``CGenerator``), with four concrete subclasses for different
traffic regimes: HeDS (empirical headway distributions), congested
(narrow-normal gaps), Poisson (exponential arrivals), and constant
(deterministic). Members on the derived classes are listed explicitly
to avoid duplicate inherited-symbol targets.

.. doxygenclass:: CFlowGenerator
   :project: pybtls
   :members: Generate, prepareNextGen, setMaxBridgeLength, getCurBlock, GenerateGap, GenerateSpeed, updateProperties, genExponential

.. doxygenclass:: CFlowGenHeDS
   :project: pybtls
   :members: GenerateGap, GenerateSpeed, updateProperties

.. doxygenclass:: CFlowGenCongested
   :project: pybtls
   :members: GenerateGap, GenerateSpeed, updateProperties

.. doxygenclass:: CFlowGenPoisson
   :project: pybtls
   :members: GenerateGap, GenerateSpeed, updateProperties

.. doxygenclass:: CFlowGenConstant
   :project: pybtls
   :members: GenerateGap, GenerateSpeed
