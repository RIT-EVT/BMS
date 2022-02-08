/**
 * For this test, BQSettings will be serialized and deserialized to ensure
 * the data is properly formatter and verify that the data is parsed
 * correctly.
 */
#include <EVT/io/manager.hpp>
#include <EVT/dev/storage/M24C32.hpp>

#include <BMS/BQSetting.hpp>
#include <BMS/BQSettingStorage.hpp>
#include <BMS/BMSLogger.hpp>
#include <BMS/dev/BQ76952.hpp>

namespace IO = EVT::core::IO;

int main() {
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);

    uart.printf("\r\n\r\nEEPROM Dump\r\n");

    EVT::core::IO::I2C& i2c = EVT::core::IO::getI2C<IO::Pin::PB_8, IO::Pin::PB_9>();
    EVT::core::DEV::M24C32 eeprom(0x50, i2c);

    BMS::LOGGER.setUART(&uart);
    BMS::LOGGER.setLogLevel(BMS::BMSLogger::LogLevel::DEBUG);

    BMS::DEV::BQ76952 bq(i2c, 0x04);
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
