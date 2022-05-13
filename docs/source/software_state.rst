=========================
State of the BMS Software
=========================

Development of the BMS software, including capturing of software system
specifications, software architecture and design, implementation, and
initial validation took place by Collin Bolles. This document details the
state of the software as of 5/13/2022 and is intended as a reference for the
next developer to take up the BMS project. The remainder of the document will
go over the current software architecture, important implementation details,
notable pitfalls, and what remains of the project.

Introduction
============

As stated elsewhere in the BMS documentation. The BMS system is built around
the BQ76952 which is a battery management chip created by TI. Much of the
software architecture is designed around the functionality of the BQ chip and
thus an understanding of the chip is necessary to understand the implementation
details of the software. Documentation for the BQ76952 can be found
`on TI's website <https://www.ti.com/product/BQ76952>`_.

Additionally, the BMS system from a software perspective is made up of a
STM32f334 for the microcontroller, an EEPROM module for non-volitile memory,
and a CAN transciever for network communication. These are the main components
that the software interacts with and the usage of each element is discussed
in details futher on.

Currently, the software is in a state where the basic requirements for the
system are met. Essential communication with the BQ has been implemented,
an ability to store and transfer settings is enabled, and communication on the
CAN network is possible. Additional validation is still required before the
software is considered validated for continuous use on the motorcycle system.
The currently supported features, summary of currently validated software,
and the features still needed to be added are deatiled below.

System Requirements
===================

A full indepth description of the system requirements are detailed in the
Software Requirement Specification (SRS) which can be found `here <https://dev1-bms.readthedocs.io/en/latest/srs.html>`_.
Some of the key details are listed below as they have the biggest impact on the
software.

* Transfer and storage of BQ settings
* Sharing of battery pack data over CANopen
* Control of current flow into/out of battery pack.
