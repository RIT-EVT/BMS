======================================================================
Software Requirements Specification for DEV1 Battery Management System
======================================================================


Introduction
============

Purpose
-------

The purpose of this document is to detail the software requirements and
constraints for the firmware of the Dirt Electric Vehicle 1 Battery Management
System (DEV1 BMS). This document will go into detail on the requirements
necessary for the system as well as detailing the constraints that the system
will be under. The intention is that this document will provide a means to
holistically express the needs of the DEV1 BMS system.

Document Conventions
--------------------

This document was created based on the IEEE template for software requirements
specifications. As such it will mainly adhere to the verbiage and style
set by IEEE convention. Additionally, a glossary has been provided in the
Appendix which covers common phrases that will be used in this document.

Intended Audience and Reading Suggestions
-----------------------------------------

The intention is that this document will be accessible across engineering
disciplines. The DEV1 BMS represents a highly inter-disciplinary project
on the RIT Electric Vehicle Team and as such this document should be
accessible to all involved. Below is a breakdown of the intended audiences
and how they may use the document. This list is not exhaustive, but intended
to guide common readers of this document.

* Electrical Team Members: Members of the electrical team who are designing
  and building the DEV1 BMS. Members of this group may refer to this document
  to ensure requirements and constraints align with their expectations.
  Additionally, this document can be used as a point of reference during
  the hardware/firmware bring up and debugging.
* Firmware Team Members: Developers on the firmware team who are designing,
  developing, and testing the DEV1 BMS firmware. Members of this team will
  need to refer to this document throughout the development process to ensure
  all target needs are met within the agreed upon constraints.
* Integration Team Members: RIT EVT team members who handle systems level
  integration on the team. These team members may use this document to gain
  and understanding of how the DEV1 BMS will operate. Most critically, how the
  DEV1 BMS will operate within the structure of the DEV1 architecture.

The document is laid out where it is not strictly necessary to read all
sections in their entirety. Instead each section should be self-contained
and comprehensive within its scope. As such audience members are encouraged
to read the sections that most pertain to their needs. Additionally, the
glossary in the Appendix may prove useful in situations where phrases may
be used in specific ways within this document.

Product Scope
-------------

The DEV1 BMS handles the battery management of the system. The DEV1 BMS will
provide an interface for each of the twin battery packs that will be present in
the DEV1 architecture. As such, the DEV1 BMS scope will include battery pack
health and safety, sharing battery statistics on the DEV1 CAN network,
handling cell balancing, and providing a general interface to the battery
packs. A noteworthy exclusion to the scope is the power distribution logic
which will not be handled by the DEV1 BMS. Power distribution is not
in the scope of the DEV1 BMS and instead is handled by the DEV1 Powertrain
Voltage Controller (PVC).

Safety
~~~~~~

The DEV1 BMS will handle safety-related functionality that ensures the health
of the battery pack. These range in levels from low priority to safety-critical
events. Below are the features that fall under the scope of the safety aspect
of the DEV1 BMS.

* Indicating overall battery safety
* Notification of thermal issues
* Notification of cell voltage issues
* Notification of BQ76952 errors
* Detection of battery connector

Diagnostics
~~~~~~~~~~~

The DEV1 BMS will be the main point-of-contact for the battery pack and
the cells contained within. As such, it must be able to detect and report
information on the battery pack itself. Functionality that falls under this
scope is not safety critical and is intended to be used for data collection
and decision-making during the usage of the motorcycle. This information
will be exposed on the CAN network and the DEV1 BMS will also expose means
of directly requesting data from the BQ76952 battery monitoring IC.


* Transmission of thermal state of battery pack
* Transmission of battery pack voltage
* Transmission of average cell voltage
* Exposure of BQ76952 chip

Charging
~~~~~~~~

The charging of the battery pack will have health checks that are managed by
the DEV1 BMS. After charging takes place, the DEV1 BMS will also oversee cell
balancing that will take place within the battery pack. As such, the following
features will need to be included.

* Performance of handshake with charge controller
* Reporting of health of battery pack

Overview
--------

The rest of this software design document will go further into the specifics
of the requirements while also looking at the constraints of the system. The
goal is to clarify the use cases of the DEV1 BMS and to specify what the DEV1
BMS will do in those cases. Design considerations will not be discussed,
however notable design constraints will be covered.

Overall Description
===================

Product Perspective
-------------------

The DEV1 motorcycle will make use of custom battery packs which will provide
power to the entirety of the DEV1 architecture during normal operation.
The target event for the DEV1 motorcycle is a 24-hour endurance race. With
these considerations in mind, the DEV1 BMS is critical to the success and
safety of the DEV1 project. A critical component is the safety of the
battery cells within the battery packs. As such, the DEV1 BMS will need
to perform safety checks and handle the battery cell balancing logic.

The DEV1 BMS will be a board that will be present in each battery pack.
Each DEV1 BMS will operate independently and will handle the safety of
the individual pack. This includes both during normal operation when the pack
is on the motorcycle as well as when the pack is being charged.

The DEV1 BMS itself is made up of two major configurable components. The first
is the ST microcontroller which handles programmable logic and exposes the DEV1
BMS on the CANopen network. The second is the BQ76952 battery monitor and
protector IC which handles the battery safety and monitoring logic.

User Interfaces
~~~~~~~~~~~~~~~

Users will rarely interact directly with the DEV1 BMS software. The DEV1 BMS
software will mainly be interfaced with via CANopen and thus will require
additional tools to interact with the DEV1 BMS. There is no current plan for a
team-developed tool to provide an interface for interaction with the DEV1 BMS.

Hardware Interfaces
~~~~~~~~~~~~~~~~~~~

The DEV1 BMS will be exposed on the CANopen network which is made up of
a two-wire differential pair. The connector pinout is similar to the EVT
standard but includes the OK signal in addition to CAN and power. The battery
packs also have integrated 14-pin JTAG connectors that expose SWD and UART.

Software Interfaces
~~~~~~~~~~~~~~~~~~~

The main software interface will be the expose of the BQ76952 chip over
the CAN network. The DEV1 BMS will need a software interface for acting as
a bridge between the external actor and the BQ76952 chip. The DEV1 BMS will
need to be flexible to expose all functionality of the BQ76952 so that the
BQ76952 can be configured. While this is still in development, a UART interface
is used to achieve this functionality.

Communication Interfaces
~~~~~~~~~~~~~~~~~~~~~~~~

The main communication interface for the DEV1 BMS will be CANopen. CANopen
is built on top of the hardware and data layer specifications of CAN. The
majority of the CAN-based network communication that will be used will
conform to CANopen, including how the DEV1 BMS will expose information on
the DEV1 system network. The exposed BQ76952 chip logic may or may not
conform to CANopen, depending on the final implementation.

Communication between the ST microcontroller and the BQ7695 will be handled
via I2C. The BQ76952 contains the specifications of the I2C interface.

Memory Constraints
~~~~~~~~~~~~~~~~~~

The produced software is limited to the 64KB of flash memory that is
available on the STM32F334r8. Therefore the resulting binary must fit within
this size.

Operations
~~~~~~~~~~

From an operational perspective, the DEV1 BMS has one main output to the rest
of the vehicle: the BMS OK signal. During storage and when not in use, this
signal will be low, indicating that the BMS should not be used. When the BMS
detects that it is on the vehicle or charger and is safe to use, it will
change this signal to high, indicating that it's ready for use. If, at any
point during use, the BMS detects that its conditions for use are no longer
safe, it will disable this OK signal, indicating that the vehicle or charger
should stop running current through it. More detail on the BMS OK signal can
be found in the safety section below.

The other major output of the DEV1 BMS is data reported over the CAN network.
This data includes pack temperatures, cell voltages, current, error values,
and more. The latest EDS file for the board can be found in the EVT CAN tools
repository.

Site Adaptation
~~~~~~~~~~~~~~~

The DEV1 BMS is intended specifically for the DEV1 system. Therefore, the
software requirements and design will center around the specifics of the DEV1
system. No additional adaptations are currently being considered.

Product Functions
-----------------

Safety
~~~~~~

Control of the BMS OK Signal
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The DEV1 BMS will need the ability to detect and respond to a number of
unsafe situations that can arise during use, including over- or
under-voltage cells, high temperatures, loose connections, and more. To
ensure the vehicle and charger are safe to operate, the BMS regularly runs
several checks to verify the health of the system. It checks for

* I2C communication failures with the BQ chip
* Alarm signals from the BQ chip
* Dangerously high temperatures in the pack or on the PCB

If these checks fail at any point, the BMS will enter an error state, which
requires a specific series of CAN messages to escape. During testing, this
allows us to debug the system while it's experiencing the error. During a
race, the rider will need to put in special inputs to reset a BMS, which
makes it very clear to them that an error has occurred.

In addition to being in a healthy state, before enabling the OK signal each
time the battery is put on a vehicle or charger, the BMS checks two things:

* The high voltage connector interlock is detected
* The CANopen heartbeat of either the PVC or charge controller is detected

Assuming no health issues arise, the BMS will keep the OK signal enabled
until the interlock is no longer detected.

Notification of Thermal Issues
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The DEV1 BMS will contain thermistors for temperature sensing. They will be
used both for determining if a thermal safety critical event has taken place
and for reporting the current temperatures of the pack to the rest of the network.
This will allow other systems to react to issues with high battery temperatures
before they reach the level that the BMS disables the OK signal. Data will be
reported via the CANopen network which will be discussed in greater detail in the
section "External Interface Requirements".

Notification of Cell Voltage Issues
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The DEV1 battery pack will need to have constant health check on the
voltage of the cells. As such, the DEV1 BMS will handle collecting
and broadcasting the state of the cell voltages on the CANopen network so
that other systems can respond accordingly.

Notification of BQ76952 State
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The BQ76952 is the IC which enables the DEV1 BMS to interact with the battery
pack. As such it has the internal logic for collecting and reporting on the
health of the battery pack and the cells within. The DEV1 BMS will need to
expose the state of the BQ76952 on the CANopen network for safety response
actions and to inform the rest of the DEV1 system on the state of the
battery pack.

Detection of Battery Connector
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The DEV1 battery pack is equipped with an interlock which can be used to
detect the presence of a connector attached to the battery pack. Use of this
interlock is critical for battery operator safety. The contact points of the
battery should only be electrically connected when a valid connector is
present. Otherwise, the battery contact points should not be connected.

Diagnostics
~~~~~~~~~~~

Transmission of Thermal State of the Battery Pack
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The DEV1 BMS will continually monitor the temperature readings from inside
of the battery pack and report the temperature on the CANopen network. The
temperature data will be reported at a fixed interval.

Transmission of Battery Pack Voltage
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The DEV1 BMS will poll the BQ76952 chip to collect the voltage of the whole
pack and each of the cells individually. This data will then be published on
the CANopen network.

Exposure of BQ76952 Chip
^^^^^^^^^^^^^^^^^^^^^^^^

The BQ76952 chip is the most important component in the DEV1 BMS, as it is
responsible for monitoring current and voltage during charging and discharging
of the packs. The BQ76952 is controlled primarily by configuring a large set of
parameters. As we optimize the DEV1 system, we should continuously refine this
configuration. Therefore, the BMS will need to expose those configuration
parameters over the CANopen network.

Charging
~~~~~~~~

Performance of Handshake with Charge Controller
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Before charging can begin, the DEV1 BMS will need to enable the OK signal. As
part of the process to enable this, the BMS will perform a handshake with the
Charge Controller. The DEV1 BMS will handle making a series of health checks
that will follow the same logic as the "Control of the BMS OK Signal". If the
BMS detects the interlock and Charge Controller heartbeat and determines that
the battery pack is in a safe state for charging, it will enable the OK signal,
indicating to the Charge Controller that charging may begin. At any time during
charging, the BMS can determine that charging is no longer safe and disable the
OK signal.

Reporting of Health of Battery Pack
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

During the charging process, the DEV1 BMS will continue to output health
information on the battery pack. The data that will be sent out will follow
the specifications of the "Diagnostics" section.

User Classes and Characteristics
--------------------------------

Those who interact with the DEV1 BMS will be expected to have a high level
of understanding of the electrical system as well as having a high level
of knowledge on battery safety. Human interaction with the DEV1 BMS will
only take place during charging, data collection, and BQ76952 configuration.
For charging it is expected that at least one person familiar with the DEV1 BMS
software and hardware is present.

For data collection, less technical experience will be required as the user
should not need to interact with any safety-critical systems. During this time
an external device can be used to collect the diagnostic messages from the BMS
over the CAN network.

During configuration of the BQ76952. The users who are interacting with the
DEV1 BMS will need to be 1 firmware team member and 1 electrical member who
worked directly on the DEV1 BMS. Since the BQ76952 is a safety critical
component, a very high technical knowledge will be needed.

Operating Environments
----------------------

The software will operate on the ST microcontroller present on the DEV1 BMS.
The software environment is embedded with no operating system present. All
development will take place through the EVT-core library and will interact
directly with the ST microcontroller.

Design and Implementation Constraints
-------------------------------------

The DEV1 BMS software will exist in an embedded environment and as such,
all design considerations will require the software to be runnable on an
embedded system.

Additionally, for the low-level interactions with the ST microcontroller,
the EVT-core software library will be used. Any additional required
functionality will need to be considered and added into the EVT-core library
itself.

Communication with the rest of the DEV1 architecture will take place via
CANopen. Design of the communication system will need to revolve around
CANopen and adhere to CANopen standards.

The hardware has already been determined and the software must be designed to
support the existing hardware. The microcontroller will be a STM32F334r8 chip
and the battery monitor chip will be a BQ76952. Software design will revolve
around the limitations of those chips.

User Documentation
------------------

Documentation will need to exist that highlight the safety logic of the
DEV1 BMS. This will include a means of determining what has triggered
a safety event on the DEV1 BMS. A large part of the documentation will
focus on the object dictionary which is the main means of interacting on
the CANopen network.

Additional documentation will need to exist for how the DEV1 BMS will
expose the BQ76952. Exposure of the BQ76952 will take place over CANopen
and proper documentation will need to exist for users to be able to
configure the BQ76952.

There should also be documentation for any further debugging or maintenance
procedures used with the BMS. As one of the most complex boards our team has
developed, it will take time for new members to ramp up to working on it. If we
are able to provide more documentation, it will take new members less time to
ramp up on this project.

Constraints
-----------

Below are some constraints worth considering. They are a fixed part of
the system.

* Development must be in C/C++
* Communication will take place using CANopen
* EVT-core will be used for low level microcontroller interfacing
* Must be developed for the STM32F334r8
* Battery monitoring will take place through the BQ76952

Assumptions and Dependencies
----------------------------

It will be assumed that all systems interacting with the DEV1 BMS will read and
react properly to the BMS OK signal. It is also assumed that the BQ76952 chip
will behave exactly as described by its datasheet, detecting and reporting all
voltages, currents, temperatures, and errors accurately, unless communication
between the BQ chip and ST microcontroller fails.

Apportioning of Requirements
----------------------------

At this point in the life cycle of the DEV1 BMS project, deployment has begun.
Due to the nature of student-run teams, it is unlikely that there will be
further revision to this system, as new students will likely start new projects.
As a result, there are no future requirements planned to be added to this
system.

Specific Requirements
=====================

External Interface Requirements
-------------------------------

BQ76952 CAN Control
~~~~~~~~~~~~~~~~~~~

The BQ76952 CAN interface is an exposed ability to communicate with the
BQ76952. The CAN interface will actually expose the I2C interface that the
STM32F334r8 has with the BQ76952. This will limit software complexity and
will ensure that all the features of the BQ76952 are correctly exposed. These
messages will come across the network realistically at any point from the
perspective of the DEV1 BMS, but practically these messages will be received
only when the battery pack is not on the vehicle.

Read Request Message Format
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Externally, a CAN message can be sent to the STM32F334r8 which will be
interpreted as a request to interact with the BQ76952. Messages with
a data length of 1 will be interpreted as a read request.

CAN ID Extended: 0x800

Data Length: 1

====    ===================================================
Byte    Description
----    ---------------------------------------------------
0       Address of the register to read from of the BQ76952
====    ===================================================

Read Response Message Format
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

After a read request made, the ST microcontroller will respond with a
response message. The response message will contain a single byte that
was read from the BQ76952.

CAN ID Extended: 0x801

Data Length: 1

====    ==========================
Byte    Description
----    --------------------------
0       Byte read from the BQ76952
====    ==========================

Write Request Message Format
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A request to write to the BQ76952 can also be made. Instead of a single
byte, two bytes will be sent.

CAN ID Extended: 0x800

Data Length: 2

====    ======================================
Byte    Description
----    --------------------------------------
0       The address of the BQ76952 to write to
1       The value to write to that address
====    ======================================


Other Nonfunctional Requirements
================================

* Software will fit within 64KB of Flash memory
* Design and development will be handled by the firmware team
* Testing will take place for failure cases
* Software will need to be robust enough to handle power loss

Appendix
========

Glossary
--------

===========   ===========================================
Term          Definition
-----------   -------------------------------------------
BMS           Battery Management System
BQ76952       Battery monitor IC
CAN           Controller Area Network
CANopen       Communication protocol built on CAN
DEV1          Dirt Electric Vehicle 1
EVT           Electric Vehicle Team
I2C           Inter-Integrated Circuit
KB            Kilo-bytes
STM32F334r8   ST Microcontroller selected for this project
TMS           Temperature Management System
===========   ===========================================

References
----------

* `BQ76952 3-s to 16-s high-accuracy battery monitor and protector for Li-ion, Li-polymer and LiFePO4 <https://www.ti.com/product/BQ76952>`_
* `CANopen - The standardized embedded network <https://www.can-cia.org/canopen/>`_
* `EVT-core <https://evt-core.readthedocs.io/en/latest/>`_
* `STM32f334r8 Mainstream Mixed signals MCUs Arm Cortex-M4 core with DSP and FPU, 64 Kbytes of Flash memory, 72 MHz CPU, 12-bit ADC 5 MSPS, Comparator, Op-Amp <https://www.st.com/en/microcontrollers-microprocessors/stm32f302r8.html>`_

Revision
--------

========    ============================          ==========
Revision    Description                           Date
--------    ----------------------------          ----------
1           Initial documentation.                10/19/2021
2           Update after initial release          01/15/2024
========    ============================          ==========
