#include <BMS/dev/BQ76952.hpp>

namespace BMS::DEV {

BQ76952::BQ76952(EVT::core::IO::I2C& i2c, uint8_t i2cAddress)
    : i2c(i2c), i2cAddress(i2cAddress) {
    // Empty constructor
}

void BQ76952::writeSetting(BMS::BQSetting& setting) {
    // Call the cooresponding setting write command
    switch (setting.getSettingType()) {
        case BMS::BQSetting::BQSettingType::DIRECT:
            this->writeDirectSetting(setting);
            break;
        case BMS::BQSetting::BQSettingType::SUBCOMMAND:
            this->writeSubcommandSetting(setting);
            break;
        case BMS::BQSetting::BQSettingType::RAM:
            this->writeRAMSetting(setting);
            break;
    }
}

void BQ76952::enterConfigUpdateMode() {
    // Number of times it will wait to see if the device has entered
    // config update mode
    // static constexpr uint8_t NUM_ATTEMPTS = 10;

    uint8_t transfer[] = {0x90, 0x00};
    i2c.writeMemReg(i2cAddress, 0x3E, transfer, 2, 1, 100);

    // TODO: Wait for the 0x12 Battery Status()[CFGUPDATE] flag to set
    // to ensure the device has entered CONFIG_UPDATE mode

    // uint8_t status = 0;
    // do {
    //      status = i2c.readReg(i2cAddress, 0x12) & 0x1;
    // } while(!status);
}

void BQ76952::exitConfigUpdateMode() {
    uint8_t transfer[] = {0x90, 0x00};
    i2c.writeMemReg(i2cAddress, 0x3E, transfer, 2, 1, 100);
}

BQ76952::BQ76952Status BQ76952::makeDirectRead(uint8_t reg, uint16_t* result) {
    // Write out the target register
    auto status = i2c.write(i2cAddress, reg);
    if (status != EVT::core::IO::I2C::I2CStatus::OK) {
        return BQ76952Status::ERROR;
    }

    // Attempt to read back the value
    uint8_t resultRaw[2];
    status = i2c.read(i2cAddress, resultRaw, 2);
    if (status != EVT::core::IO::I2C::I2CStatus::OK) {
        return BQ76952Status::ERROR;
    }

    *result = resultRaw[1] << 8 | resultRaw[0];

    // Return successful
    return BQ76952Status::OK;
}

BQ76952::BQ76952Status BQ76952::makeSubcommandRead(uint16_t reg, uint32_t* result) {
    // Write out the target subcommand
    uint8_t targetReg[] = {reg & 0xFF, (reg >> 8) & 0XFF};
    auto status = i2c.writeMemReg(i2cAddress, 0x3E, targetReg, 2, 1, 1);

    // Read back from the memory
    uint8_t resultRaw[4];
    status = i2c.readMemReg(i2cAddress, 0x40, &resultRaw[0], 4, 1);
    if (status != EVT::core::IO::I2C::I2CStatus::OK) {
        return BQ76952Status::ERROR;
    }

    *result = ((resultRaw[3] & 0xFF) << 26) | ((resultRaw[2] & 0xFF) << 16) |
              ((resultRaw[1] & 0xFF) << 8)  | (resultRaw[0] & 0xFF);

    return BQ76952Status::OK;
}

BQ76952::BQ76952Status BQ76952::makeRAMRead(uint16_t reg, uint32_t* result) {

    return BQ76952Status::OK;
}

void BQ76952::writeDirectSetting(BMS::BQSetting& setting) {
    uint8_t reg = static_cast<uint8_t>(setting.getAddress());

    // At this point the data should be 16 bites in size
    uint16_t data = static_cast<uint16_t>(setting.getData());

    makeDirectCommand(reg, data);
}

void BQ76952::writeSubcommandSetting(BMS::BQSetting& setting) {}

void BQ76952::writeRAMSetting(BMS::BQSetting& setting) {
    // Array which stores all bytes that make up a RAM write request
    // transfer[0]: LSB of the RAM register address
    // transfer[1]: LSB of the address in RAM
    // transfer[2]: MSB of the address in RAM
    // transfer[3:]: Data to put into RAM in little endian
    uint8_t transfer[3 + setting.getNumBytes()];

    // transfer[0] = RAM_BASE_ADDRESS;
    transfer[0] = static_cast<uint8_t>(setting.getAddress() & 0xFF);
    transfer[1] = static_cast<uint8_t>((setting.getAddress() >> 8) & 0xFF);

    for (int i = 0; i < setting.getNumBytes(); i++) {
        transfer[2 + i] = (setting.getData() >> (i * 8)) & 0xFF;
    }

    // i2c.write(i2cAddress, transfer, 3 + setting.getNumBytes());
    i2c.writeMemReg(i2cAddress, RAM_BASE_ADDRESS, transfer,
                    2 + setting.getNumBytes(), 1, 100);

    // Calculate and write out checksum and data length,
    // checksum algorithm = ~(ram_address + sum(data_bytes))
    // Detailed in BQ76952 Software Development Guide
    uint8_t checksum = 0;
    for (int i = 1; i < (3 + setting.getNumBytes()); i++) {
        checksum += transfer[i];
    }
    checksum = ~checksum;
    uint8_t length =
        1 + 3 + setting.getNumBytes();  // Extra 1 for the I2C address itself

    // transfer[0] = RAM_CHECKSUM_ADDRESS;
    transfer[0] = checksum;
    transfer[1] = length;

    // TODO: Determine if the basic I2C write method is correct,
    // i2c.write(i2cAddress, transfer, 3 + setting.getNumBytes());
    i2c.writeMemReg(i2cAddress, RAM_CHECKSUM_ADDRESS, transfer, 2, 1, 100);
}

void BQ76952::makeDirectCommand(uint8_t registerAddr, uint16_t data) {
    uint8_t* reg = &registerAddr;
    // Data in little endian
    uint8_t bytes[] = {static_cast<uint8_t>(data & 0xFF),
                       static_cast<uint8_t>(data >> 8)};

    i2c.writeReg(i2cAddress, reg, 1, bytes, 2);
}

}  // namespace BMS::DEV
