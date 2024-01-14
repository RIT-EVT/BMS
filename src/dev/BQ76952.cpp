#include <dev/BQ76952.hpp>

#include <EVT/utils/log.hpp>
#include <EVT/utils/time.hpp>

// (void)0 is added to the end of each macro to force users to follow the macro with a ';'
/// Macro to make an I2C transfer and return an error on failure
#define BQ_I2C_RETURN_IF_ERR(func)                   \
    if (func != EVT::core::IO::I2C::I2CStatus::OK) { \
        return Status::I2C_ERROR;                    \
    }                                                \
    (void) 0

/// Macro to pass along errors that may have been generated
#define RETURN_IF_ERR(func)                                                                     \
    {                                                                                           \
        Status result_ = func;                                                                  \
        if (result_ != Status::OK) {                                                            \
            EVT::core::log::LOGGER.log(EVT::core::log::Logger::LogLevel::ERROR, "BQ ERROR: %d", \
                                       (uint8_t) result_);                                      \
            return result_;                                                                     \
        }                                                                                       \
    }                                                                                           \
    (void) 0

///////////////////////////////////////////////////////////////////////////////
/// Functions for interacting with the BQ76952 balancing logic through CANopen
///////////////////////////////////////////////////////////////////////////////
/**
 * This function is used to get the size of the balancing data. This will
 * always be a fixed size 1 byte since the state of balancing is either
 * enabled (1) or disabled (0)
 *
 * @param[in] obj The CANopen stack object dictionary, (not used here)
 * @param[in] node The CANopen stack node (not used)
 * @param[in] width The width of the data in bytes as it is stored in the
 *                  object dictionary (not used)
 * @param[in] priv Private data, pointer to the BQ76952 instance
 * @return The number of bytes representing the state of balancing (1)
 */
static uint32_t COBQBalancingSize(struct CO_OBJ_T* obj, struct CO_NODE_T* node,
                                  uint32_t width, void* priv) {

    (void) obj;
    (void) node;
    (void) width;
    (void) priv;

    return 1;
}

/**
 * Read in the balance state of the given cell. This will communicate with the
 * BQ to determine the state.
 *
 * @param[in] obj The CANopen stack object dictionary, used to determine
 *                the cell number.
 * @param[in] node The CANopen stack node (not used)
 * @param[out] buf THe buffer to populate with data
 * @param[in] len The number of bytes to read
 * @param[in] priv The private data (BQ76952 instance)
 * @return CO_ERR_NONE on success
 */
static CO_ERR COBQBalancingRead(CO_OBJ_T* obj, CO_NODE_T* node, void* buf,
                                uint32_t len, void* priv) {

    (void) node;
    (void) len;

    auto targetCell = static_cast<uint8_t>(obj->Data);

    auto* bq = (BMS::DEV::BQ76952*) priv;

    bool isBalancing = false;
    BMS::DEV::BQ76952::Status status = bq->isBalancing(targetCell, &isBalancing);

    if (status != BMS::DEV::BQ76952::Status::OK) {
        return CO_ERR_OBJ_READ;
    }

    auto* result = (uint8_t*) buf;
    *result = isBalancing;

    return CO_ERR_NONE;
}

/**
 * Write out the state of the balancing. Can be used to enable balancing by
 * providing a 1 and disable balancing by providing a 0
 *
 * @param[in] obj CANopen stack object dictionary element, used to determine
 *                the cell number.
 * @param[in] node The CANopen stack node (not used)
 * @param[in] buf Bytes containing the enable/disable state
 * @param[in] len Number of bytes in the buf (should be 1)
 * @param[in] priv The private data (BQ76952 instance)
 * @return CO_ERR_NONE on success
 */
static CO_ERR COBQBalancingWrite(CO_OBJ_T* obj, CO_NODE_T* node, void* buf,
                                 uint32_t len, void* priv) {
    (void) node;
    (void) len;

    auto targetCell = static_cast<uint8_t>(obj->Data);

    auto* bq = (BMS::DEV::BQ76952*) priv;

    uint8_t balancingState = *(uint8_t*) buf;
    balancingState = balancingState > 0 ? 1 : 0;

    BMS::DEV::BQ76952::Status status = bq->setBalancing(targetCell, balancingState);

    if (status != BMS::DEV::BQ76952::Status::OK) {
        return CO_ERR_OBJ_WRITE;
    }

    return CO_ERR_NONE;
}

/**
 * Control logic, for the balancing logic does not need to do anything
 */
static CO_ERR COBalancingCtrl(CO_OBJ* obj, CO_NODE_T* node, uint16_t func,
                              uint32_t para, void* priv) {
    (void) obj;
    (void) node;
    (void) para;
    (void) func;
    (void) priv;

    return CO_ERR_NONE;
}

namespace BMS::DEV {

BQ76952::BQ76952(EVT::core::IO::I2C& i2c, uint8_t i2cAddress)
    : balancingCANOpen{
        COBQBalancingSize,
        COBalancingCtrl,
        COBQBalancingRead,
        COBQBalancingWrite,
        this,
    },
      i2c(i2c), i2cAddress(i2cAddress) {}

BQ76952::Status BQ76952::writeSetting(BMS::BQSetting& setting) {
    // Right now, the BQ only accepts settings made into RAM
    if (setting.getSettingType() != BMS::BQSetting::BQSettingType::RAM) {
        EVT::core::log::LOGGER.log(EVT::core::log::Logger::LogLevel::ERROR, "Setting type is incorrect");
        return Status::ERROR;
    }
    return writeRAMSetting(setting);
}

BQ76952::Status BQ76952::enterConfigUpdateMode() {
    // Number of times it will wait to see if the device has entered
    // config update mode
    static constexpr uint8_t NUM_ATTEMPTS = 10;

    BQ_I2C_RETURN_IF_ERR(i2c.writeMemReg(i2cAddress, COMMAND_ADDR,
                                         const_cast<uint8_t*>(ENTER_CONFIG), 2, 1, 100));

    // Make sure the device actually entered Config Update Mode
    bool isInConfigMode = false;
    int numAttempts = 0;
    while (!isInConfigMode && numAttempts < NUM_ATTEMPTS) {
        RETURN_IF_ERR(inConfigMode(&isInConfigMode));
        numAttempts++;
    }

    if (!isInConfigMode) {
        return Status::ERROR;
    }

    return Status::OK;
}

BQ76952::Status BQ76952::exitConfigUpdateMode() {
    BQ_I2C_RETURN_IF_ERR(i2c.writeMemReg(i2cAddress, COMMAND_ADDR,
                                         const_cast<uint8_t*>(EXIT_CONFIG), 2, 1, 100));

    // Make sure the device actually exited Config Update Mode
    bool isInConfigMode;
    RETURN_IF_ERR(inConfigMode(&isInConfigMode));
    if (isInConfigMode) {
        return Status::ERROR;
    }

    return Status::OK;
}

BQ76952::Status BQ76952::makeDirectRead(uint8_t reg, uint16_t* result) {
    // Write out the target register
    BQ_I2C_RETURN_IF_ERR(i2c.write(i2cAddress, reg));

    // Attempt to read back the value
    uint8_t resultRaw[2];
    BQ_I2C_RETURN_IF_ERR(i2c.read(i2cAddress, resultRaw, 2));

    *result = resultRaw[1] << 8 | resultRaw[0];

    // Return successful
    return Status::OK;
}

BQ76952::Status BQ76952::makeSubcommandRead(uint16_t reg, uint32_t* result) {
    // Write out the target subcommand
    uint8_t targetReg[] = {static_cast<uint8_t>(reg & 0xFF), static_cast<uint8_t>((reg >> 8) & 0XFF)};
    BQ_I2C_RETURN_IF_ERR(i2c.writeMemReg(i2cAddress, COMMAND_ADDR, targetReg, 2, 1, 1));

    // Read back from the memory
    uint8_t resultRaw[4];
    BQ_I2C_RETURN_IF_ERR(i2c.readMemReg(i2cAddress, READ_BACK_ADDR, &resultRaw[0], 4, 1));

    *result = ((resultRaw[3] & 0xFF) << 26) | ((resultRaw[2] & 0xFF) << 16) | ((resultRaw[1] & 0xFF) << 8) | (resultRaw[0] & 0xFF);

    return Status::OK;
}

BQ76952::Status BQ76952::commandOnlySubcommand(uint16_t reg) {
    // Write out the target subcommand
    uint8_t targetReg[] = {static_cast<uint8_t>(reg & 0xFF), static_cast<uint8_t>((reg >> 8) & 0XFF)};
    BQ_I2C_RETURN_IF_ERR(i2c.writeMemReg(i2cAddress, COMMAND_ADDR, targetReg, 2, 1, 1));

    return Status::OK;
}

BQ76952::Status BQ76952::makeRAMRead(uint16_t reg, uint32_t* result) {
    // Write out the target subcommand
    uint8_t targetReg[] = {static_cast<uint8_t>(reg & 0xFF), static_cast<uint8_t>((reg >> 8) & 0XFF)};
    BQ_I2C_RETURN_IF_ERR(i2c.writeMemReg(i2cAddress, COMMAND_ADDR, targetReg, 2, 1, 1));

    // Read back from the memory
    uint8_t resultRaw[4];
    BQ_I2C_RETURN_IF_ERR(i2c.readMemReg(i2cAddress, READ_BACK_ADDR, &resultRaw[0], 4, 1));

    *result = static_cast<uint32_t>(resultRaw[3] & 0xFF) << 24 | static_cast<uint32_t>(resultRaw[2] & 0xFF) << 16 | static_cast<uint32_t>(resultRaw[1] & 0xFF) << 8 | static_cast<uint32_t>(resultRaw[0] & 0xFF);

    return Status::OK;
}

BQ76952::Status BQ76952::writeRAMSetting(BMS::BQSetting& setting) {
    // Array which stores all bytes that make up a RAM write request
    // transfer[0]: LSB of the address in RAM
    // transfer[1]: MSB of the address in RAM
    // transfer[2:]: Data associated with the setting
    uint8_t transfer[7];

    // Insert RAM address into transfer buffer
    transfer[0] = static_cast<uint8_t>(setting.getAddress() & 0xFF);
    transfer[1] = static_cast<uint8_t>((setting.getAddress() >> 8) & 0xFF);

    // Insert the data into the transfer buffer
    for (int i = 0; i < setting.getNumBytes(); i++) {
        transfer[2 + i] = (setting.getData() >> (i * 8)) & 0xFF;
    }

    // Send over the settings
    BQ_I2C_RETURN_IF_ERR(i2c.writeMemReg(i2cAddress, RAM_BASE_ADDR, transfer,
                                         3 + setting.getNumBytes(), 1, 100));

    // Calculate and write out checksum and data length,
    // checksum algorithm = ~(ram_address + sum(data_bytes))
    // Detailed in BQ76952 Software Development Guide
    uint8_t checksum = transfer[0] + transfer[1];
    for (int i = 2; i < (2 + setting.getNumBytes()); i++) {
        checksum += transfer[i];
    }
    checksum = ~checksum;
    uint8_t length = 4 + setting.getNumBytes();

    // transfer[0]: calculated checksum
    // transfer[1]: number of data bytes
    transfer[0] = checksum;
    transfer[1] = length;

    // Transfer the checksum and length
    BQ_I2C_RETURN_IF_ERR(i2c.writeMemReg(i2cAddress, RAM_CHECKSUM_ADDR,
                                         transfer, 2, 1, 100));

    // Verify the transfer took place successfully. From the BQ Technical
    // Reference Manual Chapter 3. This can be done by polling the address
    // register until the address matches what was written out can be read
    // back. Then you can verify the checksum and length matches what was
    // written out
    uint16_t address = 0;
    uint16_t targetAddress = setting.getAddress();
    uint16_t rawResponse;
    uint32_t startTime = EVT::core::time::millis();

    // Try to read back the address that was written out
    while (address != targetAddress) {
        // Attempt to reach back the address
        RETURN_IF_ERR(makeDirectRead(RAM_BASE_ADDR, &rawResponse));
        address = rawResponse;

        // Check to see if a timeout occurred
        if (EVT::core::time::millis() - startTime > TIMEOUT) {
            return Status::TIMEOUT;
        }
    }

    // Verify the data written matches
    uint32_t readData;
    RETURN_IF_ERR(makeRAMRead(setting.getAddress(), &readData));
    switch (setting.getNumBytes()) {
    case 1:
        readData = readData & 0xFF;
        break;
    case 2:
        readData = readData & 0xFFFF;
        break;
    case 3:
        readData = readData & 0xFFFFFF;
        break;
    case 4:
        readData = readData & 0xFFFFFFFF;
        break;
    }
    if (readData != setting.getData()) {
        return Status::ERROR;
    }

    return Status::OK;
}

BQ76952::Status BQ76952::makeDirectWrite(uint8_t registerAddr, uint16_t data) {
    uint8_t* reg = &registerAddr;
    // Data in little endian
    uint8_t bytes[] = {static_cast<uint8_t>(data & 0xFF),
                       static_cast<uint8_t>(data >> 8)};

    BQ_I2C_RETURN_IF_ERR(i2c.writeReg(i2cAddress, reg, 1, bytes, 2));

    return Status::OK;
}

BQ76952::Status BQ76952::inConfigMode(bool* result) {
    /** Bit 0 in the BATTERY_STATUS_REG is the config mode status */
    const uint8_t configMask = 0x1;

    BQ_I2C_RETURN_IF_ERR(i2c.write(i2cAddress, BATTERY_STATUS_ADDR));

    // Attempt to read back the value
    uint8_t resultRaw[2];
    BQ_I2C_RETURN_IF_ERR(i2c.read(i2cAddress, resultRaw, 2));

    *result = resultRaw[0] & configMask;
    return Status::OK;
}

BQ76952::Status BQ76952::communicationStatus() {
    uint32_t readID;
    auto result = makeSubcommandRead(0x0001, &readID);

    if (result != BQ76952::Status::OK) {
        return BQ76952::Status::ERROR;
    }

    if (readID == BQ_ID) {
        return BQ76952::Status::OK;
    }
    return BQ76952::Status::ERROR;
}

BQ76952::Status BQ76952::getCellVoltage(uint16_t cellVoltages[NUM_CELLS], uint32_t& sum, cellVoltageInfo& voltageInfo) {
    Status status = Status::OK;
    uint8_t cellVoltageReg = CELL_VOLTAGE_BASE_ADDR;
    //Must use temporary storage variables or else the values reported over CAN will be inaccurate from regular changes.
    uint32_t tempVoltage = 0;
    uint16_t tempMinVoltage = 65535;
    uint16_t tempMaxVoltage = 0;
    uint8_t tempMinCellID;
    uint8_t tempMaxCellID;

    // Loop over all the cells and update the corresponding voltage
    for (uint8_t i = 0; i < NUM_CELLS; i++) {
        status = makeDirectRead(CELL_REG(i), &cellVoltages[i]);
        if (status != Status::OK) {
            return status;
        }
        if (cellVoltages[i] < tempMinVoltage) {
            tempMinVoltage = cellVoltages[i];
            tempMinCellID = i + 1;
        } else if (cellVoltages[i] > tempMaxVoltage) {
            tempMaxVoltage = cellVoltages[i];
            tempMaxCellID = i + 1;
        }
        tempVoltage += cellVoltages[i];

        // Each cell register is 2 bytes off from each other
        cellVoltageReg += 2;
    }

    sum = tempVoltage;
    voltageInfo.minCellVoltage = tempMinVoltage;
    voltageInfo.minCellVoltageId = tempMinCellID;
    voltageInfo.maxCellVoltage = tempMaxVoltage;
    voltageInfo.maxCellVoltageId = tempMaxCellID;

    return BQ76952::Status::OK;
}

BQ76952::Status BQ76952::isBalancing(uint8_t targetCell, bool* balancing) {

    uint32_t reg = 0;
    RETURN_IF_ERR(makeRAMRead(0x83, &reg));

    uint8_t targetLocation = CELL_BALANCE_MAPPING[targetCell - 1];

    *balancing = reg >> targetLocation & 0x1;
    return Status::OK;
}

BQ76952::Status BQ76952::setBalancing(uint8_t targetCell, uint8_t enable) {
    // Read the current state, update the target cell, and write back out
    // the data
    uint32_t reg = 0;
    RETURN_IF_ERR(makeRAMRead(0x83, &reg));

    // Keep only the bottom half
    reg &= 0xFFFF;

    // Clear or set target bit
    if (enable) {
        reg |= (1 << CELL_BALANCE_MAPPING[targetCell - 1]);
    } else {
        // Ands the register value with 1s in all positions excepted the target
        reg &= ~(1 << CELL_BALANCE_MAPPING[targetCell - 1]);
    }

    // Enable host controlled balancing
    BQSetting hostControlSetting(BQSetting::BQSettingType::RAM, 1,
                                 BALANCING_CONFIG_ADDR, 0x00);
    RETURN_IF_ERR(writeRAMSetting(hostControlSetting));

    // Write out the setting
    BQSetting setting(BQSetting::BQSettingType::RAM, 2, ACTIVE_BALANCING_ADDR, reg);
    RETURN_IF_ERR(writeRAMSetting(setting));

    return Status::OK;
}

BQ76952::Status BQ76952::getCurrent(int16_t& current) {
    return makeDirectRead(0x3a, reinterpret_cast<uint16_t*>(&current));
}

BQ76952::Status BQ76952::getVoltage(uint16_t& voltage) {
    uint16_t voltageBuf;
    RETURN_IF_ERR(makeDirectRead(0x34, &voltageBuf));
    voltage = voltageBuf * 10;
    return BQ76952::Status::OK;
}

BQ76952::Status BQ76952::getTemps(BqTempInfo& bqTempInfo) {
    uint16_t buf;
    RETURN_IF_ERR(makeDirectRead(0x68, &buf));
    bqTempInfo.internalTemp = (buf - 2732) / 10;
    RETURN_IF_ERR(makeDirectRead(0x70, &buf));
    bqTempInfo.temp1 = (buf - 2732) / 10;
    RETURN_IF_ERR(makeDirectRead(0x74, &buf));
    bqTempInfo.temp2 = (buf - 2732) / 10;

    return BQ76952::Status::OK;
}

BQ76952::Status BQ76952::getBQStatus(uint8_t bqStatusArr[7]) {
    uint16_t buf;

    for (uint8_t i = 0; i < 3; i++) {
        RETURN_IF_ERR(makeDirectRead(0x02 + i * 2, &buf));
        bqStatusArr[i] = buf % 256;
    }
    RETURN_IF_ERR(makeDirectRead(0x62, &buf));
    bqStatusArr[3] = buf % 256;
    bqStatusArr[4] = buf / 256;
    RETURN_IF_ERR(makeDirectRead(0x12, &buf));
    bqStatusArr[5] = buf % 256;
    bqStatusArr[6] = buf / 256;

    return BQ76952::Status::OK;
}

}// namespace BMS::DEV
