=======
BQ76952
=======

The BQ76952 (generally referred to as the BQ chip) is the battery monitor and
management IC that is used in the DEV1 BMS. Proper usage of this IC is
absoluetly critical to the functionality of the BMS. The BMS firmware, revolves
around the BQ chip and the bulk of its features directly relate to the BQ
chip.

For references on the BQ chip specifically, refer to
`its product <https://www.ti.com/product/BQ76942?utm_source=google&utm_medium=cpc&utm_campaign=APP-BMS-null-prodfolderdynamic-cpc-pf-google-wwe&utm_content=prodfolddynamic&ds_k=DYNAMIC+SEARCH+ADS&DCM=yes&gclid=CjwKCAiA1aiMBhAUEiwACw25MdGrYL9P-UQI9mrlAoDzh5-RaOkCX3gbS8RK6OHS-VIA_KdlyK2FWxoC9AcQAvD_BwE&gclsrc=aw.ds>`_.

Below are resources for how the BMS firmware itself interacts with the BQ
chip. The main points of interaction are as follows.

1. Writing the settings to the BQ chip on power up
2. Responding to the "ALERT" pin of the BQ chip
3. Controlling high side driver through the BQ chip
4. Polling BQ chip for battery status
5. Acting as a debug interface with the BQ chip

.. toctree::
   :maxdepth: 2
   :caption: Features:

   settings_transfer.rst
