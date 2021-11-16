/**
 * For this test, BQSettings will be serialized and deserialized to ensure
 * the data is properly formatter and verify that the data is parsed
 * correctly.
 */
#include <EVT/io/manager.hpp>
#include <BMS/BQSetting.hpp>
#include <BMS/BQSettingStorage.hpp>
#include <BMS/BMSLogger.hpp>
#include <EVT/dev/storage/platform/M24C32.hpp>

namespace IO = EVT::core::IO;

constexpr uint8_t NUM_SETTINGS = 12;

int main() {
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);

    uart.printf("\r\n\r\nEEPROM Dump\r\n");

    EVT::core::IO::I2C& i2c = EVT::core::IO::getI2C<IO::Pin::PB_8, IO::Pin::PB_9>();
    EVT::core::DEV::M24C32 eeprom(0x50, i2c);

    BMS::LOGGER.setUART(&uart);
    BMS::LOGGER.setLogLevel(BMS::BMSLogger::LogLevel::DEBUG);


    BMS::BQSettingsStorage bqSettingsStorage(eeprom);
    bqSettingsStorage.resetEEPROMOffset();

    BMS::BQSetting setting;
    for (uint8_t i = 0; i < NUM_SETTINGS; i++) {
        bqSettingsStorage.readSetting(setting);

        uart.printf("Command Type: %u, Address: 0x%04X, Num Bytes: %u, Data: 0x%08X\r\n",
            setting.getSettingType(), setting.getAddress(),
            setting.getNumBytes(), setting.getData());

        bqSettingsStorage.incrementEEPROMOffset();
    }

}
