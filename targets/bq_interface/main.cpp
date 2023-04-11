/**
 * This target is used for testing the functionality of the BQ chip
 */

#include <cstdlib>

#include <EVT/io/I2C.hpp>
#include <EVT/io/UART.hpp>
#include <EVT/manager.hpp>
#include <EVT/utils/log.hpp>
#include <EVT/utils/time.hpp>

#include <BMS.hpp>
#include <dev/BQ76952.hpp>

namespace IO = EVT::core::IO;
namespace time = EVT::core::time;
namespace log = EVT::core::log;

constexpr size_t MAX_BUFF = 100;

char inputBuffer[MAX_BUFF];

/**
 * Function for making a direct read request
 *
 * @param[in] uart The UART interface to read in from
 * @param[in] bq The BQ interface to communicate with
 */
void directRead(IO::UART& uart, BMS::DEV::BQ76952& bq) {
    uart.printf("Enter the direct address in hex: 0x");
    uart.gets(inputBuffer, MAX_BUFF);
    uart.printf("\r\n");

    uint32_t reg = strtol(inputBuffer, nullptr, 16);

    uint16_t regValue = 0;
    auto result = bq.makeDirectRead(reg, &regValue);

    // Make sure the read was successful
    if (result != BMS::DEV::BQ76952::Status::OK) {
        uart.printf("Failed to read register: 0x%x\r\n", reg);
        return;
    }

    // Print the result
    uart.printf("Register 0x%x: 0x%04X\r\n", reg, regValue);
}

/**
 * Function for making a subcommand request
 *
 * @param[in] uart The UART interface to read from
 * @param[in] bq The BQ interface
 */
void subcommandRead(IO::UART& uart, BMS::DEV::BQ76952& bq) {
    uart.printf("Enter the subcommand address in hex: 0x");
    uart.gets(inputBuffer, MAX_BUFF);
    uart.printf("\r\n");

    // Determine the target subcommand register
    uint16_t reg = strtol(inputBuffer, nullptr, 16);

    // Make the read request
    uint32_t subcommandValue = 0;
    auto result = bq.makeSubcommandRead(reg, &subcommandValue);

    // Make sure the read was successful
    if (result != BMS::DEV::BQ76952::Status::OK) {
        uart.printf("Failed to read register: 0x%x\r\n", reg);
        return;
    }

    uart.printf("Register 0x%x: 0x%08X\r\n", reg, subcommandValue);
}

/**
 * Function for making a RAM read request
 *
 * @param[in] uart The UART interface to read in from
 * @param[in] bq The BQ interface to use
 */
void ramRead(IO::UART& uart, BMS::DEV::BQ76952& bq) {
    uart.printf("Enter the RAM address in hex: 0x");
    uart.gets(inputBuffer, MAX_BUFF);
    uart.printf("\r\n");

    // Determine the target subcommand register
    uint16_t reg = strtol(inputBuffer, nullptr, 16);

    // Make the read request
    uint32_t ramValue = 0;
    auto result = bq.makeRAMRead(reg, &ramValue);

    // Make sure the read was successful
    if (result != BMS::DEV::BQ76952::Status::OK) {
        uart.printf("Failed to read register: 0x%x\r\n", reg);
        return;
    }

    uart.printf("Register 0x%x: 0x%08X\r\n", reg, ramValue);
}

/**
 * Read the balancing state for a specific cell
 *
 * @param[in] uart The UART interface to read in from
 * @param[in] bq The BQ interface to use
 */
void readBalancing(IO::UART& uart, BMS::DEV::BQ76952& bq) {
    uart.printf("Enter the cell to read balancing of: ");
    uart.gets(inputBuffer, MAX_BUFF);
    uart.printf("\r\n");

    uint8_t targetCell = strtol(inputBuffer, nullptr, 10);

    bool isBalancing;
    if (bq.isBalancing(targetCell, &isBalancing) != BMS::DEV::BQ76952::Status::OK) {
        uart.printf("Failed to read balancing state\r\n");
    }

    uart.printf("%d balancing state: %d\r\n", targetCell, isBalancing);
}

/**
 * Set the balancing state for the specific cell
 *
 * @param[in] uart The UART interface to read from
 * @param[in] bq The BQ interface to use
 */
void setBalancing(IO::UART& uart, BMS::DEV::BQ76952& bq) {
    uart.printf("Enter the cell to set balancing of: ");
    uart.gets(inputBuffer, MAX_BUFF);
    uart.printf("\r\n");

    uint8_t targetCell = strtol(inputBuffer, nullptr, 10);

    uart.printf("Enter the target state (0 or 1): ");
    uart.gets(inputBuffer, MAX_BUFF);
    uart.printf("\r\n");

    uint8_t targetState = strtol(inputBuffer, nullptr, 10);

    if (bq.setBalancing(targetCell, targetState) != BMS::DEV::BQ76952::Status::OK) {
        uart.printf("Failed to set the state of balancing\r\n");
    }
}

/**
 * Function for making a direct write request
 *
 * @param[in] uart The UART interface to write in from
 */
void directWrite(IO::UART& uart) {}

/**
 * Function for making an indirect write request
 *
 * @param[in] uart The UART interface to write in from
 */
void indirectWrite(IO::UART& uart) {}

/**
 * Function for making a RAM write request
 *
 * NOTE: BQ chip must be in config mode
 *
 * @param[in] uart The UART interface to write in from
 */
void ramWrite(IO::UART& uart, BMS::DEV::BQ76952& bq) {
    bool inConfigMode;
    auto result = bq.inConfigMode(&inConfigMode);
    if (result != BMS::DEV::BQ76952::Status::OK) {
        uart.printf("Failed to get if the BQ is in config update mode\r\n");
        return;
    }

    if (!inConfigMode) {
        uart.printf(
            "Cannot write RAM settings unless the BQ is in config update "
            "mode\r\n");
        return;
    }

    const BMS::BQSetting::BQSettingType type =
        BMS::BQSetting::BQSettingType::RAM;

    uart.printf("Number of bytes in setting: 0x");
    uart.gets(inputBuffer, MAX_BUFF);
    uart.printf("\r\n");

    uint8_t numBytes = strtol(inputBuffer, nullptr, 16);

    uart.printf("RAM address: 0x");
    uart.gets(inputBuffer, MAX_BUFF);
    uart.printf("\r\n");

    uint16_t ramAddress = strtol(inputBuffer, nullptr, 16);

    uart.printf("Data to write: 0x");
    uart.gets(inputBuffer, MAX_BUFF);
    uart.printf("\r\n");

    uint32_t data = strtol(inputBuffer, nullptr, 16);

    uart.printf("RAM Setting to Send\r\n");
    uart.printf("\tNumber of bytes: 0x%02X\r\n", numBytes);
    uart.printf("\tRAM Address: 0x%04X\r\n", ramAddress);
    uart.printf("\tData: 0x%08X\r\n", data);

    uart.printf("Send command (y/n): ");
    char command = uart.getc();
    uart.gets(inputBuffer, MAX_BUFF);
    uart.printf("\r\n");

    if (command == 'y') {
        uart.printf("Sending RAM setting\r\n");

        BMS::BQSetting setting(type, numBytes, ramAddress, data);

        result = bq.writeRAMSetting(setting);

        if (result != BMS::DEV::BQ76952::Status::OK) {
            uart.printf("Failed to write out RAM setting\r\n");
            return;
        }

        uart.printf("Setting written out\r\n");
        return;
    } else {
        uart.printf("Cancelling RAM setting\r\n");
        return;
    }
}

/**
 * Put the BQ chip into config mode and check the status
 *
 * @param[in] uart The interface to print status messages to
 * @param[in] bq The interface to communicate with the BQ
 */
void enterConfigMode(IO::UART& uart, BMS::DEV::BQ76952& bq) {
    uart.printf("Putting the BQ chip into config mode\r\n");

    // Attempt to put the BQ into configure update mode
    auto result = bq.enterConfigUpdateMode();
    if (result != BMS::DEV::BQ76952::Status::OK) {
        uart.printf("Failed writing out config update mode\r\n");
        return;
    }

    uart.printf("BQ in config mode\r\n");
}

/**
 * Have the BQ chip exit config update mode
 *
 * @param[in] uart The interface to print status messages to
 * @param[in] bq The interface to communicate with the BQ
 */
void exitConfigMode(IO::UART& uart, BMS::DEV::BQ76952& bq) {
    uart.printf("Pulling the BQ chip out of config mode\r\n");

    // Attempt to put the BQ into configure update mode
    auto result = bq.exitConfigUpdateMode();
    if (result != BMS::DEV::BQ76952::Status::OK) {
        uart.printf("Failed writing out config update mode\r\n");
        return;
    }

    uart.printf("BQ not in config mode\r\n");
}

void commandOnlySub(IO::UART& uart, BMS::DEV::BQ76952& bq) {
    uart.printf("Enter the command-only subcommand address in hex: 0x");
    uart.gets(inputBuffer, MAX_BUFF);
    uart.printf("\r\n");

    // Determine the target subcommand register
    uint16_t reg = strtol(inputBuffer, nullptr, 16);

    // Run the command
    auto result = bq.commandOnlySubcommand(reg);

    // Make sure the read was successful
    if (result != BMS::DEV::BQ76952::Status::OK) {
        uart.printf("Failed to read register: 0x%x\r\n", reg);
        return;
    }

    uart.printf("Register 0x%x run\r\n", reg);
}

void getVoltages(IO::UART& uart, BMS::DEV::BQ76952& bq) {
    uint16_t tot = 0;
    for (uint8_t i = 0; i < 16; i++) {
        uint8_t reg = 0x14 + 2 * i;
        uint16_t regValue = 0;
        auto result = bq.makeDirectRead(reg, &regValue);

        // Make sure the read was successful
        if (result != BMS::DEV::BQ76952::Status::OK) {
            uart.printf("Failed to read register: 0x%x\r\n", reg);
            return;
        }

        uart.printf("Cell %2d Voltage, Register %#x: %d.%03d\r\n", i + 1, reg, regValue / 1000, regValue % 1000);

        tot += regValue;
    }
    uart.printf("Total: %d.%d", tot / 1000, tot % 1000);
}

void transferSettings(IO::UART& uart, BMS::DEV::BQ76952& bq, EVT::core::DEV::M24C32 eeprom) {
    uart.printf("Really transfer settings? (y/n): ");
    uart.gets(inputBuffer, MAX_BUFF);
    uart.printf("\r\n");

    if (inputBuffer[0] == 'y') {
        uart.printf("Transferring settings...\r\n");
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
        uart.printf("All settings transferred");
    } else {
        uart.printf("Settings transfer cancelled");
    }
}

void getTemperatures(IO::UART& uart, BMS::DEV::BQ76952& bq, BMS::DEV::ThermistorMux tMux) {
    for (uint8_t i = 0; i < 6; i++) {
        uart.printf("Thermistor %d ADC counts: %d\r\n", i, tMux.getTemp(i));
    }
    uint16_t result;
    bq.makeDirectRead(0x68, &result);
    result -= 2732;
    uart.printf("BQ Internal Temp: %d.%01d\r\n", result / 10, result % 10);
    bq.makeDirectRead(0x70, &result);
    result -= 2732;
    uart.printf("BQ Board Temp 1: %d.%01d\r\n", result / 10, result % 10);
    bq.makeDirectRead(0x74, &result);
    result -= 2732;
    uart.printf("BQ Board Temp 2: %d.%01d\r\n", result / 10, result % 10);
}

void getInterlock(IO::UART& uart, BMS::DEV::Interlock interlock) {
    uart.printf("Interlock Detected: %s\r\n", interlock.isDetected() ? "true" : "false");
}

void getAlarm(IO::UART& uart, IO::GPIO& alarm) {
    uart.printf("Alarm Set: %s\r\n", alarm.readPin() == IO::GPIO::State::HIGH ? "true" : "false");
}

void setOK(IO::UART& uart, IO::GPIO& bmsOK) {
    uart.printf("Set OK pin (0/1): ");
    uart.printf("\r\n");
    uart.gets(inputBuffer, MAX_BUFF);
    if (inputBuffer[0] == '1') {
        bmsOK.writePin(IO::GPIO::State::HIGH);
        uart.printf("Set BMS OK high\r\n");
    } else {
        bmsOK.writePin(IO::GPIO::State::LOW);
        uart.printf("Set BMS OK low\r\n");
    }
}

int main() {
    EVT::core::platform::init();

    IO::I2C& i2c = IO::getI2C<BMS::BMS::I2C_SCL_PIN, BMS::BMS::I2C_SDA_PIN>();
    BMS::DEV::BQ76952 bq(i2c, 0x08);
    EVT::core::DEV::M24C32 eeprom(0x57, i2c);

    IO::UART& uart = IO::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(115200, true);
    log::LOGGER.setUART(&uart);
    log::LOGGER.setLogLevel(log::Logger::LogLevel::DEBUG);


    IO::ADC& adc = IO::getADC<BMS::BMS::TEMP_INPUT_PIN>();

    IO::GPIO& muxs1 = IO::getGPIO<BMS::BMS::MUX_S1_PIN>();
    IO::GPIO& muxs2 = IO::getGPIO<BMS::BMS::MUX_S2_PIN>();
    IO::GPIO& muxs3 = IO::getGPIO<BMS::BMS::MUX_S3_PIN>();
    IO::GPIO* muxPinArr[3] = { &muxs1, &muxs2, &muxs3 };

    BMS::DEV::ThermistorMux tmux(muxPinArr, adc);

    IO::GPIO& interlockGPIO = IO::getGPIO<BMS::BMS::INTERLOCK_PIN>(IO::GPIO::Direction::INPUT);
    BMS::DEV::Interlock interlock(interlockGPIO);

    IO::GPIO& alarm = IO::getGPIO<BMS::BMS::ALARM_PIN>(IO::GPIO::Direction::INPUT);

    IO::GPIO& bmsOK = IO::getGPIO<BMS::BMS::OK_PIN>(IO::GPIO::Direction::OUTPUT);

    time::wait(500);

    while (true) {
        uart.printf("\r\nEnter command: ");
        // Read in the command
        char command = uart.getc();
        uart.gets(inputBuffer, MAX_BUFF);
        uart.printf("\r\n");

        switch (command) {
        // Direct read
        case 'd':
            directRead(uart, bq);
            break;
        // Subcommand read
        case 's':
            subcommandRead(uart, bq);
            break;
        // RAM read
        case 'r':
            ramRead(uart, bq);
            break;
        // Direct write
        case 'D':
            break;
        // Subcommand write
        case 'S':
            ramWrite(uart, bq);
            break;
        // RAM write
        case 'R':
            ramWrite(uart, bq);
            break;
        // Enter config mode
        case 'c':
            enterConfigMode(uart, bq);
            break;
        // Exit config mode
        case 'x':
            exitConfigMode(uart, bq);
            break;
        case 'b':
            readBalancing(uart, bq);
            break;
        case 'B':
            setBalancing(uart, bq);
            break;
        case 'f':
            commandOnlySub(uart, bq);
            break;
        case 'v':
            getVoltages(uart, bq);
            break;
        case 't':
            transferSettings(uart, bq, eeprom);
            break;
        case 'T':
            getTemperatures(uart, bq, tmux);
            break;
        case 'i':
            getInterlock(uart, interlock);
            break;
        case 'a':
            getAlarm(uart, alarm);
            break;
        case 'o':
            setOK(uart, bmsOK);
            break;
        }
    }

    return 0;
}
