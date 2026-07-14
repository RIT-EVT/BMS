/**
 * This test is to explore the ability to transfer settings from EEPROM to the
 * BQ chip.
 */

#include <core/dev/storage/M24C32.hpp>
#include <core/manager.hpp>
#include <core/utils/log.hpp>
#include <core/utils/time.hpp>

#include <BMS.hpp>
#include <BQSetting.hpp>
#include <BQSettingStorage.hpp>
#include <dev/BQ76952.hpp>

namespace io = core::io;
namespace log = core::log;

constexpr uint8_t BQ_I2C_ADDR = 0x08;

int main() {
    core::platform::init();

    io::UART& uart = io::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(115200);
    io::I2C& i2c = io::getI2C<BMS::BMS::I2C_SCL_PIN, BMS::BMS::I2C_SDA_PIN>();
    dev::M24C32 eeprom(0x57, i2c);

    uart.printf("\r\n\r\nBQ Setting Transfer Test\r\n");

    log::LOGGER.setUART(&uart);
    log::LOGGER.setLogLevel(log::Logger::LogLevel::DEBUG);

    core::time::wait(500);

    io::GPIO& bqReset = io::getGPIO<BMS::BMS::BQ_RESET_PIN>();
    BMS::dev::BQ76952 bq(i2c, 0x08, bqReset);
    BMS::BQSettingsStorage settingsStorage(eeprom, bq);

    bool isComplete = false;
    settingsStorage.resetTransfer();
    while (!isComplete) {
        auto status = settingsStorage.transferSetting(isComplete);

        switch (status) {
        case BMS::dev::BQ76952::Status::ERROR:
            uart.printf("FAILED: BQ specific error\r\n");
            break;
        case BMS::dev::BQ76952::Status::I2C_ERROR:
            uart.printf("FAILED: I2C error\r\n");
            break;
        case BMS::dev::BQ76952::Status::TIMEOUT:
            uart.printf("FAILED: Timeout waiting for BQ\r\n");
            break;
        case BMS::dev::BQ76952::Status::OK:
            uart.printf("SUCCESS\r\n");
            break;
        default:
            uart.printf("FAILED: Unknown error\r\n");
            break;
        }
    }

    core::time::wait(500);

    uart.printf("Setting transfer complete");
}
