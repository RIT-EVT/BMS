=========================
Test setup for BMS v4.0
=========================
This document will cover how to setup a test environment for the BMS v4.0.

Getting Started
===========
To get started, the settings for the BMS need to transferred, follow the steps outlined in :ref:`transfer_over_uart`. This is a **very** important step and must be done **once** when a board is first brought up.

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

If there are any errors during settings transfer, unplug and replug the board. Reflash ``bq_interface`` and try again. Errors in this section are caused by a flaw in the BMS v4.0 that fails to hold the BQ reset line low. When the reset line is held high, it will shutdown the BQ chip. This flaw is present on BMS 1.0

Reset line is not pulled low
if the reset line is held in the wrong place for too long it will shutdown the BQ chip


Testing
============
You are now ready to start testing BMS functions.

Testing Polarity / Current
============
Set current limit on power supply all the way down

ps + -> hs_curr_p (red)    = charging
ps - -> hs_curr_n (purple) = charging

ps + -> hs_curr_n (purple) = discharging
ps - -> hs_curr_p (red)    = discharging

N +0.08mv: 0xFD0A, 0xFD09, -718, -759
P +0.08mv: 0x02CE, 0x02DA

P > N = +
P < N = -

Set balance state to 1, point heat gun and see if it gets hot
