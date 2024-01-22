#pragma once

#include <EVT/io/I2C.hpp>

#include "BMSInfo.hpp"
#include <BQSetting.hpp>

#include <Canopen/co_obj.h>

#define CELL_REG(i) CELL_BALANCE_MAPPING[i] * 2 + 0x14

namespace BMS::DEV {

/**
 * Represents the functionality that the BQ76952 contains. This is a layer
 * of abstraction which is used for handling the I2C communication between
 * the host and the BQ chip.
 *
 * Part of the logic contained is the ability to write out BQ settings to the
 * BQ chip itself. This will be able to handle taking in settings and making
 * the corresponding I2C commands to write out the settings.
 * TI Technical Reference Manual: https://www.ti.com/lit/ug/sluuby2b/sluuby2b.pdf
 */
class BQ76952 {
public:
    /// The number of cells connected to the BQ chip
    static constexpr uint8_t NUM_CELLS = 12;

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
        OK = 0x00,
        TIMEOUT = 0x10,
        I2C_ERROR = 0x20,
        ERROR = 0x40,
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
     * @param[out] result The result of the read request
     * @return The status of the read request attempt
     */
    Status makeSubcommandRead(uint16_t reg, uint32_t* result);

    /**
     * Runs a subcommand that has no result
     *
     * @param reg The subcommand register address
     * @return The status of the subcommand attempt
     */
    BQ76952::Status commandOnlySubcommand(uint16_t reg);

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
     *         Status::OK => Successfully wrote out the setting
     */
    Status writeRAMSetting(BQSetting& setting);

    /**
     * Check to see if the BQ chip is in configure mode. Configure mode is
     * a state where the BQ chip is able to handle making settings changes.
     * This mode is discussed in detail in the BQ datasheet.
     *
     * @param[out] result Populated with true if the BQ chip is in config mode
     * @return The status of attempting to get the current mode
     *          Status::I2C_ERROR => Failed to communicate with the BQ
     *          Status::ERROR => BQ encountered an error attempting to read
     *          Status::OK => Configure mode state read successfully
     */
    Status inConfigMode(bool* result);

    /**
     * Attempt to see if the BQ chip can be communicated with.
     *
     * This will read the ID of the BQ chip, and verify it matches the
     * expected value.
     *
     * @return State::OK on success, Status::ERROR otherwise
     */
    Status communicationStatus();

    /**
     * Fill a buffer with the per cell voltage.
     *
     * @param[out] cellVoltages The buffer to fill with the cell voltage. Must be NUM_CELLS in size
     * @param[out] sum The total voltage across all cells
     * @param[out] voltageInfo A struct containing the values below.
     * @return The status of the read attempt
     */
    Status getCellVoltage(uint16_t cellVoltages[NUM_CELLS], uint32_t& sum, CellVoltageInfo& voltageInfo);

    /**
     * Determine the state of balancing on a given cell. This will read the
     * balancing state from the BQ and report back.
     *
     * @param[in] targetCell The target cell to check the balancing of
     * @param[out] balancing Updates to represents if the cell is balancing
     * @return The state of the read attempt
     */
    Status isBalancing(uint8_t targetCell, bool* balancing);

    /**
     * Write out the balancing state to the target cell. Writing a 1 enables
     * balancing, writing a 0 disables balancing
     *
     * @param[in] targetCell The target cell to change the balance state of
     * @param[in] enable 1 for enabling balancing 0 otherwise
     * @return The state of the write attempt
     */
    Status setBalancing(uint8_t targetCell, uint8_t enable);

    /**
     *
     * @param current
     * @return
     */
    Status getCurrent(int16_t& current);

    /**
     *
     * @param voltage
     * @return
     */
    Status getVoltage(uint16_t& voltage);

    /**
     *
     * @param bqTempInfo
     * @return
     */
    Status getTemps(BqTempInfo& bqTempInfo);

    /**
     *
     * @param bqStatusArr
     * @return
     */
    Status getBQStatus(uint8_t bqStatusArr[7]);

    /** Total voltage read by the BQ chip (measured in milliVolts) */
    uint32_t totalVoltage = 0;

    /** CANopen interface for probing the state of the balancing */
    CO_OBJ_TYPE balancingCANOpen;

private:
    /** Used for commands and subcommands */
    static constexpr uint8_t COMMAND_ADDR = 0x3E;
    static constexpr uint8_t READ_BACK_ADDR = 0x40;

    /** Keep track of various states of the BQ chip */
    static constexpr uint8_t BATTERY_STATUS_ADDR = 0x12;
    static constexpr uint8_t RAM_BASE_ADDR = 0x3E;
    static constexpr uint8_t RAM_CHECKSUM_ADDR = 0x60;

    /** Base address where the cell voltages are located */
    static constexpr uint8_t CELL_VOLTAGE_BASE_ADDR = 0x14;

    /** Addresses for controlling balancing */
    static constexpr uint16_t BALANCING_CONFIG_ADDR = 0x9335;
    static constexpr uint16_t ACTIVE_BALANCING_ADDR = 0x0083;

    /** Used to enter and exit config mode */
    static constexpr uint8_t ENTER_CONFIG[2] = {0x90, 0x00};
    static constexpr uint8_t EXIT_CONFIG[2] = {0x92, 0x00};

    static constexpr uint8_t CELL_REGS[] = {
        0x14,
        0x16,
        0x18,
        0x1A,
        0x1C,
        0x1E,
        0x20,
        0x24,
        0x28,
        0x2C,
        0x30,
        0x32,
    };

    /**
     * Contains a mapping between the target cell and the corresponding
     * location in the `CB_ACTIVE_CELLS` bitmap. The idea that each cell is
     * an index into this lookup table.
     * NOTE: Cells are numbered starting at 1, so to get the bit position
     * for the first cell (cell 1) use index 0 (cell number - 1)
     */
    static constexpr uint8_t CELL_BALANCE_MAPPING[] = {
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        8,
        10,
        12,
        14,
        15,
    };

    /** Timeout waiting to read values from the BQ76952 in milliseconds */
    static constexpr uint8_t TIMEOUT = 10;

    /** The name of the BQ chip that should be stored in the BQ chip */
    static constexpr uint16_t BQ_ID = 0x7695;

    /** I2C bus to communicate over */
    EVT::core::IO::I2C& i2c;
    /** The address of the BQ76952 on the I2C bus */
    uint8_t i2cAddress;
};

}// namespace BMS::DEV
