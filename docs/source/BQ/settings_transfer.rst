=========================
BQ76952 Settings Transfer
=========================

The BQ settings represent the "direct commands", "subcommands", and RAM
settings that can be sent to the BQ chip. The specifics of the how to send
the settings are detailed in the
`BQ Software Development Guide <https://www.ti.com/lit/an/sluaa11b/sluaa11b.pdf?ts=1636599197514&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FBQ76952>`_.

Approach to Configuring the BQ
==============================

The BQ chip has two main means of loading settings. First, the settings can
be programmed into the chip's non-violotile memory through a process called
OTP (one time programming). Those settings can only be programmed in once.
Below is a snippet from the BQ Technical Reference Manual.

    The OTP memory in the BQ76952 device is initially all-zeros. Each bit can be
    left as a "0" or written to a "1," but it cannot be written from a "1" back to
    a "0." The OTP memory includes two full images of the Data Memory configuration
    settings. At power-up, the device will XOR each setting in the first OTP image
    with the corresponding setting in the second OTP image and with the default
    value for the corresponding setting, with the resulting value stored into the
    RAM register for use during operation. This allows any setting to be changed
    from the default value using the first image, then changed back to the default
    once using the second image. The OTP memory also includes a 16-bit signature,
    which is calculated over most of the settings and stored in OTP. When the
    device is powered up, it will read the OTP settings and check that the
    signature matches that stored, to provide robustness against bit errors in
    reading or corruption of the memory. If a signature error is detected, the
    device will boot into the default configuration (as if the OTP is cleared).

Since OTP is most reliably accomplished in a manufacturing setting, it was
decided to not rely on OTP and instead send over the settings manually via
I2C commands at system startup. This is a common practice and another recommend
approach seen elsewhere in the TI documentation. For the BMS approach, it
was decided that the BMS firmware would handle the storage of the desired
settings and the programming of those settings over I2C to the BQ chip.

Storage of BQ Settings
======================
