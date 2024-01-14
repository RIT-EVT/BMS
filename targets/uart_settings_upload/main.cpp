/**
 * This utility target is used to upload settings to the BMS EEPROM via UART, so they can then be
 * transferred to the BQ chip
*/

#include <BMS.hpp>
#include <EVT/dev/storage/M24C32.hpp>
#include <EVT/manager.hpp>

namespace IO = EVT::core::IO;

int main() {
    // Initialize system
    EVT::core::platform::init();

    IO::UART& uart = IO::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(9600, true);

    IO::I2C& i2c = IO::getI2C<BMS::BMS::I2C_SCL_PIN, BMS::BMS::I2C_SDA_PIN>();
    EVT::core::DEV::M24C32 eeprom(0x57, i2c);

    uart.printf("Test start\r\n");
    uint8_t buf[2];
    uart.readBytes(buf, 2);
    uint16_t numSettings = (((uint16_t) buf[1]) << 8) + buf[0];
    eeprom.writeHalfWord(0, numSettings);
    uart.write(0);

    uint8_t setBuf[7];
    uint32_t address = 2;
    for (uint16_t i = 0; i < numSettings; i++) {
        uart.readBytes(setBuf, 7);
        eeprom.writeBytes(address, setBuf, 7);
        address += 7;
        uart.write(0);
    }
    uart.printf("Done");
}
