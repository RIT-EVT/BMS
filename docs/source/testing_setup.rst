=========================
Test setup for BMS v4.0
=========================
This document will cover how to setup a test environment for the BMS v4.0.

Getting Started
===========
To get started, the settings for the BMS need to transferred, follow the steps outlined in :ref:`_transfer_over_uart`. This is a **very** important step and must be done **once** when a board is first brought up.

Loading BMS settings
------------
Once the settings have been transferred to the BMS, you must transfer them from the STM to the BQ chip. Since we do not freeze the settings on the BQ, we are required to flash the settings **every** time the board is turned on. This is often a source of errors, and settings transfers should always be redone if other errors appear on the BMS.

.. _flash_bms:
Flashing the BMS
============
The testing setup for the BMS uses the ``bq_interface`` target. This should be built, and flashed to the BMS. Flashing should be done using an ST-Link and STM32CubeProgrammer. This is the same `same process <https://sites.google.com/g.rit.edu/evt-home-page/firmware-team/getting-started/running-code>`_ as the rest of our boards.

UART Connection
-------------
UART on the bms uses a baud rate of `115200`. When you first connect to the BMS after flashing ``bq_interface` you may not have anything in your uart terminal. To fix this, just type h and hit enter to bring up the help menu.

Settings Transfer
-------------
Before you can begin testing BMS functions you need to transfer the settings, this can be done by entering `t` and waiting for all settings to transfer.

If there are any errors during settings transfer, unplug and replug the board, reflash ``bq_interface`` and try again. Errors in this section are caused by a flaw in BMS v4.0 that fails to hold the BQ reset line low. When the reset line is held high, it will shutdown the BQ chip.

Testing
============
You are now ready to start testing BMS functions! For each section, check the values that the BMS reports, then check the real world value. After checking the real world value, recheck that the BMS has not changed (just a sanity check).

Testing Voltages
------------
Testing pack voltages is the first step in validating the BMS. First plug the BMS into a test battery pack. You can then use ``v - Read voltages`` to list out the voltages for every cell.

*Note that on BMS v4.0, the BMS skips cells 8, 10, 12, and 14. Practically, this just requires you to adjust cell numbering after 8 when checking voltages*

To double check that the BMS is correctly reading voltages, grab a multimeter with two probes. The voltage of the first cell has a different measurement process than all other cells. This is because every cell is grounded to the previous cell. Since Cell #1 doesn't have a previous cell you need to use the Batt - pad on the BMS as ground.

The diagram below shows the placement for the positive and negative probes from your multimeter.

.. image:: ./_static/images/cell_1_voltage_measurement.png
   :align: center

For every other cell (greater than 1), this diagram shows the positions of the positive and negative probes of the multi meter.

.. image:: ./_static/images/cell_voltage_measurement.png
   :align: center

Test pack balances
------------
To check cell balancing


Testing Polarity / Current
------------


Set current limit on power supply all the way down

ps + -> hs_curr_p (red)    = charging
ps - -> hs_curr_n (purple) = charging

ps + -> hs_curr_n (purple) = discharging
ps - -> hs_curr_p (red)    = discharging

direct read @ 0x3a

N +0.08mv: 0xFD0A, 0xFD09, -718, -759
P +0.08mv: 0x02CE, 0x02DA


P > N = +
P < N = -

Set balance state to 1, point thermal gun and see if it gets hot
