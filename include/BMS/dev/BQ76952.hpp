#pragma once

#include <BMS/BQSetting.hpp>
#include <EVT/io/I2C.hpp>

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
     * Represent the status reflecting the state of the BQ76952
     *
     * This can be used to represent the state of the BQ76952 or the
     * result of a BQ76952 operation.
     *
     * OK => Operation took place successfully
     * TIMEOUT => Timeout waiting for an operation
     * I2C_ERROR => Failed at the I2C level to communicate with the BQ
     * ERROR => Failed the specific BQ operation (not I2C related)
     */
    enum class Status {
        OK = 0,
        TIMEOUT = 1,
        I2C_ERROR = 2,
        ERROR = 3,
    };

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
    Status writeSetting(BQSetting& setting);

    /**
     * Enter CONFIG_UPDATE mode. This is the mode that the BQ chip should
     * be in whenever modifying settings. If settings are modified, and the
     * BQ is not in CONFIG_UPDATE mode, the results are unpredictable.
     *
     * For more information, see Section 7.6 of the BQ76952 Technical
     * Reference Manual.
     *
     * @param The result of attempting to enter config update mode
     */
    Status enterConfigUpdateMode();

    /**
     * Exit CONFIG_UPDATE mode.
     *
     * @return The result of attempting to exit config update mode
     */
    Status exitConfigUpdateMode();

    /**
     * Execute direct read request.
     *
     * Will make a direct read request to the BQ chip.
     *
     * @param[in] reg The I2C register address to read from
     * @param[out] result The data that was read
     * @return The status of the read request attempt
     */
    Status makeDirectRead(uint8_t reg, uint16_t* result);

    /**
     * Execute a subcommand read request.
     *
     * Will make a subcommand read request to the BQ chip.
     *
     * @param[in] reg The subcommand register address
     * @param[out] The result of the read request
     * @return The status of the read request attempt
     */
    Status makeSubcommandRead(uint16_t reg, uint32_t* result);

    /**
     * Execute RAM read request.
     *
     * Will make a RAM read request to the BQ chip
     *
     * @param[in] reg The RAM register to read from
     * @param[out] result The data that was read
     * @return The status of the read request
     */
    Status makeRAMRead(uint16_t reg, uint32_t* result);

    /**
     * Execute a direct command write. This involves writing out at
     * most 16 bytes to a register.
     *
     * @param[in] reg The I2C register address to write to
     * @param[in] data The data to write out
     * @return The status of the write request attempt
     */
    Status makeDirectWrite(uint8_t reg, uint16_t data);

    /**
     * Write out a subcommand settings. Subcommands take in a 16 bit address
     * which are written out to I2C registers 0x3E and 0x3F in little endian.
     *
     * Data associated with the command is written into 0x40-0x44 also in
     * little endian.
     *
     * @param[in] setting The setting to write out
     * @return The result of the setting write attempt
     *         Status::I2C_ERROR => Failed to communicate with the BQ
     *         Status::ERROR => Setting not accepted by BQ
     *         Status::OK => Succesfully wrote out the setting
     */
    Status writeRAMSetting(BQSetting& setting);

    /**
     * Check to see if the BQ chip is in configure mode. Configure mode is
     * a state where the BQ chip is able to handle making settings changes.
     * This mode is discussed in detail in the BQ datasheet.
     *
     * @param[out] result Populated with true if the BQ chip is in config mode
     * @return The status of attemping to get the current mode
     *          Status::I2C_ERROR => Failed to communicate with the BQ
     *          Status::ERROR => BQ encountered an error attempting to read
     *          Status::OK => Configure mode state read successfully
     */
    Status inConfigMode(bool* result);

    // Total voltage read by the BQ chip (measured in millivolts)
    uint32_t totalVoltage;

private:
    /** Keep track of various states of the BQ chip */
    static constexpr uint8_t BATTERY_STATUS_REG = 0x12;
    static constexpr uint8_t RAM_BASE_ADDRESS = 0x3E;
    static constexpr uint8_t RAM_CHECKSUM_ADDRESS = 0x60;

    /** Timeout waiting to read values from the BQ76952 in milliseconds */
    static constexpr uint8_t TIMEOUT = 10;

    /** I2C bus to communicate over */
    EVT::core::IO::I2C& i2c;
    /** The address of the BQ76952 on the I2C bus */
    int8_t i2cAddress;
};

}// namespace BMS::DEV
