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
