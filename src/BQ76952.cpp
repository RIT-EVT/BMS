#include <BMS/BQ76952.hpp>


namespace BMS {

BQ76952::BQ76952(EVT::core::IO::I2C& i2c, uint8_t address) :
    i2c(i2c),
    address(address) { }

void BQ76952::sendSetting(BQSetting& setting) {

    if (setting.getSettingType() == BQSetting::BQSettingType::RAM) {

        // Transfer includes the following
        // transfer[0]: RAM transfer location (0x3E)
        // transfer[1]: LSB of RAM address
        // transfer[2]: MSB of RAM address
        // transfer[3]: LSB of data
        // transfer[4]: MSB of data
        uint8_t transfer[5];

        transfer[0] = RAM_TRANSFER_LOCATION;
        transfer[1] = setting.getAddress() & 0xFF;
        transfer[2] = (setting.getAddress() >> 8) 0xFF;
        transfer[3] = setting.getData() & 0xFF;
        transfer[4] = (setting.getData() >> 8) & 0xFF;

        // Send the setting
        i2c.write(address, transfer, 5);
    }
}

}  // namespace BMS
