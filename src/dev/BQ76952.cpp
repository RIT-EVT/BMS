#include <BMS/dev/BQ76952.hpp>

namespace BMS::DEV {

BQ76952::BQ76952(EVT::core::IO::I2C& i2c, uint8_t i2cAddress) : i2c(i2c), i2cAddress(i2cAddress) {
    // Empty constructor
}

void BQ76952::writeSetting(BMS::BQSetting& setting) {
    // Call the cooresponding setting write command
    switch(setting.getSettingType()) {
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

void BQ76952::writeDirectSetting(BMS::BQSetting& setting) {
    uint8_t reg = static_cast<uint8_t>(setting.getAddress());

    // At this point the data should be 16 bites in size
    uint16_t data = static_cast<uint16_t>(setting.getData());

    makeDirectCommand(reg, data);

}

void BQ76952::writeSubcommandSetting(BMS::BQSetting& setting) {

}

void BQ76952::writeRAMSetting(BMS::BQSetting& setting) {
    // Array which stores all bytes that make up a RAM write request
    // transfer[0]: LSB of the RAM register address
    // transfer[1]: LSB of the address in RAM
    // transfer[2]: MSB of the address in RAM
    // transfer[3:]: Data to put into RAM in little endian
    uint8_t transfer[3 + setting.getNumBytes()];

    transfer[0] = RAM_BASE_ADDRESS;
    transfer[1] = static_cast<uint8_t>(setting.getAddress() & 0xFF);
    transfer[2] = static_cast<uint8_t>((setting.getAddress() >> 8) & 0xFF);

    for(int i = 0; i < setting.getNumBytes(); i++) {
        transfer[3 + i] = (setting.getData() >> (i * 8)) & 0xFF;
    }

    i2c.write(i2cAddress, transfer, 3 + setting.getNumBytes());

    // Calculate and write out checksum and data length,
    // checksum algorithm = ~(ram_address + sum(data_bytes))
    // Detailed in BQ76952 Software Development Guide
    uint8_t checksum = 0;
    for(int i = 1; i < (3 + setting.getNumBytes()); i++) {
        checksum += transfer[i];
    }
    checksum = ~checksum;
    uint8_t length = 1 + 3 + setting.getNumBytes(); // Extra 1 for the I2C address itself

    transfer[0] = RAM_CHECKSUM_ADDRESS;
    transfer[1] = checksum;
    transfer[2] = length;

    i2c.write(i2cAddress, transfer, 3);
}

void BQ76952::makeDirectCommand(uint8_t registerAddr, uint16_t data) {
    uint8_t* reg = &registerAddr;
    // Data in little endian
    uint8_t bytes[] = { static_cast<uint8_t>(data & 0xFF), static_cast<uint8_t>(data >> 8) };

    i2c.writeReg(i2cAddress, reg, 1, bytes, 2);
}

}  // namespace BMS::DEV
