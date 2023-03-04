/**
 * For this test, BQSettings will be serialized and deserialized to ensure
 * the data is properly formatter and verify that the data is parsed
 * correctly.
 */

#include <EVT/dev/storage/M24C32.hpp>
#include <EVT/io/manager.hpp>
#include <EVT/utils/log.hpp>

#include <BMS.hpp>
#include <BQSetting.hpp>
#include <BQSettingStorage.hpp>
#include <dev/BQ76952.hpp>

namespace IO = EVT::core::IO;
namespace log = EVT::core::log;

int main() {
    IO::UART& uart = IO::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(9600);

    uart.printf("\r\n\r\nEEPROM Dump\r\n");

    IO::I2C& i2c = IO::getI2C<BMS::BMS::I2C_SCL_PIN, BMS::BMS::I2C_SDA_PIN>();
    EVT::core::DEV::M24C32 eeprom(0x57, i2c);

    log::LOGGER.setUART(&uart);
    log::LOGGER.setLogLevel(log::Logger::LogLevel::DEBUG);

    BMS::DEV::BQ76952 bq(i2c, 0x08);
    BMS::BQSettingsStorage bqSettingsStorage(eeprom, bq);
    bqSettingsStorage.resetEEPROMOffset();

    // Print the number of settings read in from EEPROM
    uart.printf("Total settings: %u\r\n", bqSettingsStorage.getNumSettings());

    BMS::BQSetting setting;
    uint16_t numSettings = bqSettingsStorage.getNumSettings();
    for (uint16_t i = 0; i < numSettings; i++) {
        bqSettingsStorage.readSetting(setting);

        uart.printf("Command Type: %u, Address: 0x%04X, Num Bytes: %u, Data: 0x%08X\r\n",
                    setting.getSettingType(), setting.getAddress(),
                    setting.getNumBytes(), setting.getData());
    }
}
