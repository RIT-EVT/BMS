/**
 * This test is to explore the ability to transfer settings from EEPROM to the
 * BQ chip.
 */
#include <EVT/dev/storage/M24C32.hpp>
#include <EVT/io/manager.hpp>
#include <EVT/utils/time.hpp>

#include <BMS/BMSLogger.hpp>
#include <BMS/BQSetting.hpp>
#include <BMS/BQSettingStorage.hpp>
#include <BMS/dev/BQ76952.hpp>

namespace IO = EVT::core::IO;

constexpr uint8_t BQ_I2C_ADDR = 0x08;

int main() {
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);
    IO::I2C& i2c = IO::getI2C<IO::Pin::PB_8, IO::Pin::PB_9>();
    EVT::core::DEV::M24C32 eeprom(0x50, i2c);

    uart.printf("\r\n\r\nBQ Setting Transfer Test\r\n");

    BMS::LOGGER.setUART(&uart);
    BMS::LOGGER.setLogLevel(BMS::BMSLogger::LogLevel::DEBUG);

    EVT::core::time::wait(500);

    BMS::DEV::BQ76952 bq(i2c, BQ_I2C_ADDR);
    BMS::BQSettingsStorage settingsStorage(eeprom, bq);

    bool isComplete = false;
    settingsStorage.resetTranfer();
    uint16_t count = 0;
    while(!isComplete) {
        auto status = settingsStorage.transferSetting(isComplete);

        switch (status) {
        case BMS::DEV::BQ76952::Status::ERROR:
            uart.printf("FAILED: BQ specific error\r\n");
            break;
        case BMS::DEV::BQ76952::Status::I2C_ERROR:
            uart.printf("FAILED: I2C error\r\n");
            break;
        case BMS::DEV::BQ76952::Status::TIMEOUT:
            uart.printf("FAILED: Timeout waiting for BQ\r\n");
            break;
        case BMS::DEV::BQ76952::Status::OK:
            uart.printf("SUCCESS\r\n");
            break;
        default:
            uart.printf("FAILED: Unknown error\r\n");
            break;
        }
    }

    EVT::core::time::wait(500);

    uart.printf("Setting transfer complete");
}
