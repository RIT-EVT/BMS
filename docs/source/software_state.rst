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

These key requirements dictate how the software is designed and constrained.
An overview of each and how the software as impacted based on the requirements
are included below.

Transfer and Storage of BQ Settings
-----------------------------------

The BQ chip is designed to be able to be used in a wide range of battery
systems and as such has a lot of settings which control the functionality of
the chip. In total there are about 274 settings which control everything
from information on the battery pack itself, to fault handling cases, to
communication protocol settings. These settings are set mostly by the
Electrical team as they have the most domain knowledge in that area.

BQ Settings Overview
^^^^^^^^^^^^^^^^^^^^

The BQ supports two main ways of getting the settings into the BQ's memory.
The first approach is through a method called OTP (one time programming)
where the BQ is placed in a special state and the settings are written into
non-volitile memory. The issue which this approach, as the name suggests,
this can only be done once. As the documentation states "once a bit is set to
a 1, it cannot be set back to a 0". As we have a need to tweak settings as we
learn more and make changed to the system we cannot rely on OTP.

The second approach, and the method we use, is to transfer the settings to the
BQ on system startup. Each setting can be written to the BQ via I2C commands
so when the system starts up, the STM writes each setting to the BQ. The
benefit of this approach is that we can change the settings as need be, but the
downside is the need to store all the settings and transfer the settings to the
BQ each time the system starts up. This means that the settings have to be
stored in some non-volitile memory, and that the BMS system needs some method
to allow for the settings to be transfered from some host system to the
STM.

Transfer of Settings to BMS System
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There needs to be an approach to get the settings transferred from some host
to the BMS system. This is handled via CANopen which allows for transfer of
larger portions of data via a method called "block transfer". The code for
handling the transfer logic can be found in ``BQSettingStorage``.

The ``BQSettingStorage`` class handles the logic of exposing the setting
transfer and storage logic. It has two main responsibilities. First, the
class contains the logic of exposing a means to transfer the settings from the
host to the BMS system over CANopen. Second, the class handles reading and
writing those settings between non-volitle memory (EEPROM) to the BQ chip
itself. The transfer of the settings take place across two steps. First
the external host writes the new number of settings to the BMS system over
CANopen. Second, the external host then writes out each setting over
CANopen. The ``BQSettingStorage`` handles the whole process of updating the
number of settings and the settings themselved into EEPROM. Below is a
small sequence diagram showing what takes place to allow the settings to be
transferred.

.. image:: ./_static/image/bq_settings_update_sequence.png
   :width: 600
   :align: center

The exposure of these settings is handled via CANopen stack, so for a more
indepth understanding of how to implement custom settings you can refer
to `CANopen stack's documentation <https://canopen-stack.org/v4.2/>`_.

Converting BQStudio Settings to Binary
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The BQ settings are usually set using a TI provided software, BQStudio, and then
exported to a CSV format. The CSV format stores a number of pieces of
information including the location in the BQ where the setting should be
stored, how many bytes the setting takes up, a human readable representation
of the data, and an equation to convert the human readable format into
what can actually be stored in the BQ. These settings needs to be converted
from the CSV into a binary file which can then be transferred over CANopen.
For more information on how the data is packed into a binary format,
refer to the `setting transfer documentation <https://dev1-bms.readthedocs.io/en/latest/BQ/settings_transfer.html>`_. The backed format in that document is how the
settings are stored both for transfer over CANopen and for storage in EEPROM.

A python script is provided which handles the logic of converting the CSV
into a binary format and another script exists for the logic to transfer
the binary file over CANopen. Documentation for how to use those scripts are
included with the scripts. Luckily the process of sending a binary file over
CANopen is a standard practice, so the binary file can be transferred with
any tool capable of CANopen including a Vector CAN adapter.

The scripts to convert the CSV and transfer the CSV over CANopen can be found
in ``tools/bqsettings/``. The usage of the scripts are further explained there.

Transfer of Settings to BQ
^^^^^^^^^^^^^^^^^^^^^^^^^^

When the BMS system starts up, the STM reads the number of settings from
EEPROM and transfers that number of settings from EEPROM to the BQ over
I2C. These settings are transferred one-by-one until all have been sent across.
This takes place over a 30-45 second period.


Sharing of Battery Pack data Over CANopen
-----------------------------------------

The battery pack information is exposed over CANopen via the object dictionary.
Most of the data is polled from the BQ over I2C at some interval and a pointer
to that data is included in the CANopen objection dictionary. This sharing of
data is highly standard and does not have BMS specific logic. The currently
supported data which is exposed is listed below.

* Total battery pack voltage
* Individual cell voltage
* Current state of the BMS (based on the BMS state machine)
* Information on the state of cell balancing

Some information which is not yet exposed but should be is listed below.

* Temperature readings
* Number of BQ settings stored
* Ability to read back stored BQ settings
* Misc BQ status
