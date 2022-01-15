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

}

void BQ76952::makeDirectCommand(uint8_t registerAddr, uint16_t data) {
    uint8_t* reg = &registerAddr;
    // Data in little endian
    uint8_t bytes[] = { static_cast<uint8_t>(data & 0xFF), static_cast<uint8_t>(data >> 8) };

    i2c.writeReg(i2cAddress, reg, 1, bytes, 2);
}

}  // namespace BMS::DEV
