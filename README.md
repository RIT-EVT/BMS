# DEV1-BMS

## Introduction

### DEV1-BMS

The DEV1-BMS is the Battery Management System for the RIT-EVT Dirt
Electric Vehicle (DEV1). The system is based around the BQ76952 TI battery
monitor and protector chip. The firmware for the DEV1-BMS is written for
the STM32f302r8 microcontroller that is designed into the DEV1-BMS.

The DEV1-BMS will have the following responsibilities.

1. Run safety checks during DEV1 operation
2. Facilitate battery charging
3. Act as an interface to the on-board BQ76952
4. Expose the system over the DEV1 CAN network

For a more detailed description of the requirements, refer to the
[DEV1-BMS Software Requirements Specification](https://dev1-bms.readthedocs.io/en/latest/srs.html).

For more information on the BMS over all and the software level API,
refer to the
[Read the Docs DEV1-BMS page](https://dev1-bms.readthedocs.io/en/latest/index.html).
