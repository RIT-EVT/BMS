=======
BQ76952
=======

The BQ76952 (generally referred to as the BQ chip) is the battery monitor and
management IC that is used in the DEV1 BMS. Proper usage of this IC is
absolutely critical to the functionality of the BMS. The BMS firmware revolves
around the BQ chip and the bulk of its features directly utilize to the BQ
chip.

For references on the BQ chip specifically, refer to
`its product <https://www.ti.com/product/BQ76952>`_.

Below are resources for how the BMS firmware itself interacts with the BQ
chip. The main points of interaction are as follows:

* Writing the settings to the BQ chip on power up
* Responding to the "ALERT" pin of the BQ chip
* Polling BQ chip for battery information
* Acting as a debug interface with the BQ chip

.. toctree::
   :maxdepth: 2
   :caption: Features:

   settings_transfer.rst
   transfer_utility.rst
