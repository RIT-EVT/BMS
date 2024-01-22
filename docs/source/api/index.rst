===
API
===

This documentation outlines the library created for the BMS.

DEV
===
Devices, representation of hardware that can be interfaced with. In
general, devices are communicated with via some sort of IO interface, but that
is not strictly a rule. An LED is a simplistic example of a device.

BQ76952
-------
.. doxygenclass:: BMS::DEV::BQ76952
   :members:

Interlock
---------
.. doxygenclass:: BMS::DEV::Interlock
   :members:

ThermistorMux
-------------
.. doxygenclass:: BMS::DEV::ThermistorMux
   :members:

Top-Level Classes
=================

The rest of the classes used in this codebase are not meant to represent
hardware interfaces. They are just used to separate BMS behaviors into separate
units to help understanding and organizing the code.

BMS
---
.. doxygenclass:: BMS::BMS
   :members:

BQSetting
---------
.. doxygenclass:: BMS::BQSetting
   :members:

BQSettingStorage
----------------
.. doxygenclass:: BMS::BQSettingStorage
   :members:

ResetHandler
------------
.. doxygenclass:: BMS::ResetHandler
   :members:

SystemDetect
------------
.. doxygenclass:: BMS::SystemDetect
   :members:

Structures
==========

To make passing data between certain ``BMS`` functions more compact, some
structs have been defined.

CellVoltageInfo
---------------
.. doxygenstruct:: BMS::CellVoltageInfo
    :members:

PackTempInfo
------------
.. doxygenstruct:: BMS::PackTempInfo
    :members:

BqTempInfo
----------
.. doxygenstruct:: BMS::BqTempInfo
    :members:
