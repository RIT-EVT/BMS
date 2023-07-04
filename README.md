# DEV1 BMS

## Introduction

### DEV1 BMS

The DEV1 BMS is the Battery Management System for the RIT-EVT Dirt
Electric Vehicle (DEV1). The system is based around the BQ76952 TI battery
monitor and protector chip. The firmware for the DEV1 BMS is written for
the STM32f334r8 microcontroller that is designed into the DEV1 BMS.

The DEV1 BMS will have the following responsibilities.

1. Run safety checks during DEV1 operation
2. Facilitate battery charging
3. Act as an interface to the on-board BQ76952
4. Expose the system over the DEV1 CAN network

For a more detailed description of the requirements, refer to the
[DEV1 BMS Software Requirements Specification](https://dev1-bms.readthedocs.io/en/latest/srs.html).

For more information on the BMS over all and the software level API,
refer to the
[Read the Docs DEV1 BMS page](https://dev1-bms.readthedocs.io/en/latest/index.html).

### Documentation

Documentation is handled via Sphinx. To build the documentation, navigate
to the `docs/` folder and run `make html`. You can then open the generated
`docs/build/html/index.html`.

To generate a PDF of the Software Requirements Specification, run the command
`rinoh docs/source/srs.rst --output docs`. You can then view the PDF version
of the SRS in `docs/srs.pdf`. The SRS is identical to the one generated via
`make html`.

### Related Projects

The DEV1 BMS is one component of the larger DEV1 project, you can find related
projects in the [RIT-EVT](https://github.com/RIT-EVT) Github page.
Additionally, the STM32f334 driver code is handled via the
[EVT-core](https://github.com/RIT-EVT/EVT-core) project.
