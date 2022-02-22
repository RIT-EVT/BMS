#include <stdlib.h>
#include <string.h>

#include <BMS/dev/BQ76952.hpp>
#include <EVT/io/I2C.hpp>
#include <EVT/io/UART.hpp>
#include <EVT/io/manager.hpp>
#include <EVT/io/pin.hpp>

namespace IO = EVT::core::IO;

constexpr size_t MAX_BUFF = 100;

char inputBuffer[MAX_BUFF];

/**
 * Function for making a direct read request
 *
 * @param[in] uart The UART interface to read in from
 * @param[in] bq The BQ interface to communicate with
 */
void directRead(IO::UART &uart, BMS::DEV::BQ76952 &bq) {
    uart.printf("Enter the direct address in hex: 0x");
    uart.gets(inputBuffer, MAX_BUFF);

    uint32_t reg = strtol(inputBuffer, nullptr, 16);

    uint8_t regValue = 0;
    auto result = bq.makeDirectRead(reg, &regValue);

    // Make sure the read was successful
    if (result != BMS::DEV::BQ76952::BQ76952Status::OK) {
        uart.printf("Failed to read register: 0x%x\r\n", reg);
        return;
    }

    // Print the result
    uart.printf("Register 0x%x: 0x%x\r\n", reg, regValue);
}

/**
 * Function for making an indirect read request
 *
 * @param[in] uart The UART interface to read in from
 */
void indirectRead(IO::UART &uart) {}

/**
 * Function for making a RAM read request
 *
 * @param[in] uart The UART interface to read in from
 */
void ramRead(IO::UART &uart) {}

/**
 * Function for making a direct write request
 *
 * @param[in] uart The UART interface to write in from
 */
void directWrite(IO::UART &uart) {}

/**
 * Function for making an indirect write request
 *
 * @param[in] uart The UART interface to write in from
 */
void indirectWrite(IO::UART &uart) {}

/**
 * Function for making a RAM write request
 *
 * @param[in] uart The UART interface to write in from
 */
void ramWrite(IO::UART &uart) {}

int main() {
    IO::I2C &i2c = IO::getI2C<IO::Pin::PB_8, IO::Pin::PB_9>();
    BMS::DEV::BQ76952 bq(i2c, 0x10);

    IO::UART &uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);

    while (true) {
        // Read in the command
        char command = uart.getc();

        switch (command) {
            // Direct read
            case 'd':
                directRead(uart, bq);
                break;
            // Indirect read
            case 'i':
                break;
            // RAM read
            case 'r':
                break;
            // Direct write
            case 'D':
                break;
            // Indirect write
            case 'I':
                break;
            // RAM write
            case 'R':
                break;
        }
    }

    return 0;
}
