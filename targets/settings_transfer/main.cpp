/**
 * This test is to explore the ability to transfer settings from EEPROM to the
 * BQ chip.
 */

#include <EVT/dev/storage/M24C32.hpp>
#include <EVT/io/manager.hpp>
#include <EVT/utils/log.hpp>
#include <EVT/utils/time.hpp>

#include <BMS.hpp>
#include <BQSetting.hpp>
#include <BQSettingStorage.hpp>
#include <dev/BQ76952.hpp>

namespace IO = EVT::core::IO;
namespace log = EVT::core::log;

constexpr uint8_t BQ_I2C_ADDR = 0x08;

int main() {
    IO::init();

    IO::UART& uart = IO::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(9600);
    IO::I2C& i2c = IO::getI2C<BMS::BMS::I2C_SCL_PIN, BMS::BMS::I2C_SDA_PIN>();
    EVT::core::DEV::M24C32 eeprom(0x57, i2c);

    uart.printf("\r\n\r\nBQ Setting Transfer Test\r\n");

    log::LOGGER.setUART(&uart);
    log::LOGGER.setLogLevel(log::Logger::LogLevel::DEBUG);

    EVT::core::time::wait(500);

    BMS::DEV::BQ76952 bq(i2c, BQ_I2C_ADDR);
    BMS::BQSettingsStorage settingsStorage(eeprom, bq);

    bool isComplete = false;
    settingsStorage.resetTransfer();
    while (!isComplete) {
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
