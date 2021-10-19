======================================================================
Software Requirements Specification for DEV1 Battery Management System
======================================================================


Introduction
============

Purpose
-------

The purpose of this document is to detail the software requirements and
constraints for the firmware of the Dirt Electric Vehicle Battery Management
System (DEV1 BMS). This document will go into detail on the requirements
necessary for the system as well as detailing the contrains that the system
will be under. The intention is that this document will provide a means to
holistically express the needs to the DEV1 BMS system.

Document Conventions
--------------------

This document was created based on the IEEE template for software requirements
specifications. As such it will mainly adhere to the verbage and style
set by IEEE convention. Additonally, a glossary has been provided in the
Appendix which covers common phrases that will be used in this document.

Intended Audience and Reading Suggestions
-----------------------------------------

The intention is that this document will be accessible across engineering
disiplines. The DEV1 BMS represents a highly inter-disiplinary project
on the RIT Electric Vehicle Team and as such this document should be
accessible to all involved. Below is a break down of the intended audiences
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
  all target needs are met within the agreed upon contraints.
* Integration Team Members: RIT EVT team members who handle systems level
  integration on the team. These team members may use this document to gain
  and understanding of how the DEV1 BMS will operate. Most critically, how the
  DEV1 BMS will operate within the structure of the DEV1 architecture.

The document is layed out where it is not strictly necessary to read all
sections in their entirety. Instead each section should be self-contained
and self-entirety within its scope. As such audience members are encouraged
to read the sections that most pertain to their needs. Additionally, the
glossary in the Appendix may prove useful in situations where phrases may
be used in specific ways within this document.

Product Scope
-------------

The DEV1 BMS handles the battery management of the system. The DEV1 BMS will
interact directly with the twin battery packs that will be present in the
DEV1 architecture. As such, the DEV1 BMS scope will include battery pack
health and safety, sharing battery statistics on the DEV1 CAN network,
handling cell balancing, and providing a general interface to the battery
packs. A noteworthy exclusion to the scope is the power distribution logic
which will not be handled by the DEV1 BMS. The power distribution is not
in the scope of the DEV1 BMS and instead is handled by the DEV1 APM.

References
----------


Overall Description
===================

Product Perspective
-------------------

The DEV1 motorcycle will make use of custom battery packs which will provide
power to the entirety of the DEV1 architecture during normal operation.
The target event for the DEV1 motorcycle is a 24 hour endurance race. With
these considerations in mind, the DEV1 BMS is critical to the success and
safety of the DEV1 project. A critical component is the safety of the
battery cells within the battery packs. As such, the DEV1 BMS will need
to preform safety checks and handle the battery cell balancing logic.

The DEV1 BMS will be a board that will be present in each battery pack.
Each DEV1 BMS will operate independently and will handle the safety of
the individual pack. This includes both during normal operation when the pack
is on the motorcycle as well as when the pack is being charged.

Product Functions
-----------------

System Startup
~~~~~~~~~~~~~~

System startup refers to when the DEV1 BMS is being powered up and


User Classes and Characteristics
--------------------------------

Operating Environments
----------------------

Design and Implementation Constraints
-------------------------------------

User Documentation
------------------

Assumptions and Dependencies
----------------------------

External Interface Requirements
===============================

User Interfaces
---------------

Hardware Interfaces
-------------------

Software Interfaces
-------------------

Communication Interfaces
------------------------

System Features
===============

Other Nonfunctional Requirements
================================

Appendix
========

Glossary
--------
