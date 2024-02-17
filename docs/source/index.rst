.. Documentation master file, created by
   sphinx-quickstart on Sat Sep  4 10:17:15 2021.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

===============================
Welcome to the DEV1 BMS documentation!
===============================

Introduction
============

The DEV1 BMS is the Battery Management System for the RIT-EVT Dirt
Electric Vehicle 1 (DEV1). The system is based around the BQ76952 TI battery
monitor and protector chip. The firmware for the DEV1 BMS is written for
the STM32f334 microcontroller that is designed into the DEV1 BMS.

The DEV1 BMS will have the following responsibilities:

1. Run safety checks during DEV1 operation
2. Facilitate battery charging
3. Set up and monitor the on-board BQ76952
4. Expose the system over the DEV1 CAN network

For a more detailed description of the requirements, refer to the
`DEV1 BMS Software Requirements Specification
<https://dev1-bms.readthedocs.io/en/latest/srs.html>`_.

The DEV1 BMS project is in its deployment phase. The hardware has been
validated and integrated into the six battery packs. The software has been
tested in isolation and in test runs on the bike with the whole DEV1 system. A
detailed description of the current state of the BMS software is linked below.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   api/index.rst
   srs.rst
   software_state.rst
   BQ/index.rst

State Transition Diagram
------------------------
For a look at each stage of the BMS firmware in action. The state diagram
below can be used.

.. image:: ./_static/images/BMS_state_transition.png
   :align: center

The functionality of the BMS can be broken down into 4 major components.
The "Startup" logic which involves getting the settings for the BQ chip via
CANopen, those settings to EEPROM, and finally to the BQ chip itself. The
"Ready" state is where the BMS spends the majority of its time, not connected
to anything. In this state, it checks on the health of the BMS system, and
waits for the pack to be connected either to the pack or the charger. In the
"Charging" state, the BMS mediates charging the batteries. Finally, in the
"Power Delivery" state, the BMS is handles delivering power. The logic for
transitioning between the different states is detailed in the diagram above.


EVT-core
========

EVT-core is the RIT-EVT-produced library for interfacing with the STM32f3xx
family of microcontrollers used on the DEV1 BMS and other PCBs made by the
team. For more information on that library, visit
`RTD for EVT-core <https://evt-core.readthedocs.io/en/latest/>`_.

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
