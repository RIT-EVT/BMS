#ifndef _BQ76952_
#define _BQ76952_

#include <EVT/io/I2C.hpp>

#include <BMS/BQSetting.hpp>

namespace BMS::DEV {

/**
 * Represents the functionality that the BQ76952 contains. This is a layer
 * of abstraction which is used for handling the I2C communication between
 * the host and the BQ chip.
 *
 * Part of the logic contained is the ability to write out BQ settings to the
 * BQ chip itself. This will be able to handle taking in settings and making
 * the cooresponding I2C commands to write out the settings.
 */
class BQ76952 {
public:
    /**
     * Create a new instance of the BQ76952 which will communicate over the
     * given I2C bus with the given address.
     *
     * @param i2c[in] I2C interface to use to communicate on the bus
     * @param i2cAddress[in] The address of the BQ76952 to use
     */
    BQ76952(EVT::core::IO::I2C& i2c, uint8_t i2cAddress);

    /**
     * Write out the given settings.
     *
     * @param setting[in] The BQ setting to write out.
     */
    void writeSetting(BMS::BQSetting& setting);

private:
    static constexpr uint8_t RAM_BASE_ADDRESS = 0x3E;
    static constexpr uint8_t RAM_CHECKSUM_ADDRESS = 0x60;

    /** I2C bus to communicate over */
    EVT::core::IO::I2C& i2c;
    /** The address of the BQ76952 on the I2C bus */
    int8_t i2cAddress;

    /**
     * Write out a direct settings. Direct settings are controls that are
     * written into a single I2C register which is a byte wide
     *
     * Direct settings can have a maximum of 16 bites of data.
     *
     * @param setting[in] The direct setting to write out
     */
    void writeDirectSetting(BMS::BQSetting& setting);

    /**
     * Write out a subcommand settings. Subcommands take in a 16 bit address
     * which are written out to I2C registers 0x3E and 0x3F in little endian.
     *
     * Data associated with the command is written into 0x40-0x44 also in
     * little endian.
     */
    void writeSubcommandSetting(BMS::BQSetting& setting);

    /**
     * Write out a Data Memory Setting otherwise known as a RAM setting.
     */
    void writeRAMSetting(BMS::BQSetting& setting);

    /**
     * Make a direct command. A direct command is made by making a write
     * request to a 7-bit address with at most 2 bytes of data.
     *
     * @param address[in] 7 bit address
     * @param data[in] 16 byte data to write out
     */
    void makeDirectCommand(uint8_t address, uint16_t data);
};

}

#endif
