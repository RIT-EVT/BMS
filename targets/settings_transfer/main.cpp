/**
 * This test is to explore the ability to transfer settings from EEPROM to the
 * BQ chip.
 */
#include <EVT/io/manager.hpp>
#include <EVT/utils/time.hpp>
#include <EVT/dev/storage/platform/M24C32.hpp>

#include <BMS/dev/BQ76952.hpp>
#include <BMS/BQSetting.hpp>
#include <BMS/BQSettingStorage.hpp>

namespace IO = EVT::core::IO;

constexpr uint8_t BQ_I2C_ADDR = 0x04;

int main() {
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);
    IO::I2C& i2c = IO::getI2C<IO::Pin::PB_8, IO::Pin::PB_9>();
    EVT::core::DEV::M24C32 eeprom(0x50, i2c);

    uart.printf("\r\n\r\nBQ Setting Transfer Test\r\n");

    BMS::DEV::BQ76952 bq(i2c, BQ_I2C_ADDR);
    BMS::BQSettingsStorage settingsStorage(eeprom, bq);

    settingsStorage.transferSettings();

    EVT::core::time::wait(500);

    uart.printf("Setting transfer complete");
}
