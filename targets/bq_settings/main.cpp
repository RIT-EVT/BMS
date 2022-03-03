/**
 * For this test, BQSettings will be serialized and deserialized to ensure
 * the data is properly formatter and verify that the data is parsed
 * correctly.
 */
#include <EVT/io/manager.hpp>
#include <BMS/BQSetting.hpp>


namespace IO = EVT::core::IO;

/** Print the content of the array in a user friendly manner */
void printArray(IO::UART& uart, uint8_t* buffer, uint8_t size) {
    uart.printf("{ ");
    for (uint8_t i = 0; i < size; i++)
        uart.printf("0x%2x ", buffer[i]);
    uart.printf("}");
}

/** Check the contents of one array is the same as the other */
bool arraysEqual(uint8_t* first, uint8_t* second, uint8_t size) {
    for (uint8_t i = 0; i < size; i++)
        if (first[i] != second[i])
            return false;
    return true;
}

/**
 * First test, ensure data can be serialized properly. Will provide an
 * array with known contents to the BQSettings and ensure the parsed results
 * match expectation.
 */
void deserializeTest(IO::UART& uart) {
    /**
     * Command Byte: Direct command with 1 byte of data
     * Address: 0x0001
     * Data: 0xAA
     */
    uint8_t knownValues[] = { 0x04, 0x01, 0x00, 0xAA, 0x00, 0x00, 0x00 };
    BMS::BQSetting setting;
    setting.fromArray(knownValues);

    // Check setting type
    if (setting.getSettingType() != BMS::BQSetting::BQSettingType::DIRECT) {
        uart.printf("Deserialization FAILED, invalid setting type, got %u, expected, %u\r\n",
                setting.getSettingType(), BMS::BQSetting::BQSettingType::DIRECT);
        return;
    }

    // Check address
    if (setting.getAddress() != 0x0001) {
        uart.printf("Deserialization FAILED, invalid address, got 0x%04x, expected, 0x%04x\r\n",
                setting.getAddress(), 0x0001);
        return;
    }

    // Check number of bytes
    if (setting.getNumBytes() != 1) {
        uart.printf("Deserialization FAILED, invalid number of bytes, got %u, expected, %u\r\n",
                setting.getNumBytes(), 1);
        return;
    }

    // Check the data itself
    if (setting.getData() != 0xAA) {
        uart.printf("Deserialization FAILED, invalid data, got 0x%2x, expected 0x%2x\r\n",
                setting.getData(), 0xAA);
        return;
    }

    uart.printf("Successful Deserialization\r\n");
}

/**
 * Second test, ensure that the data can be correctly turned into an array.
 * Will make a settings value, convert it into an array, and compare it against
 * the expected output.
 */
void serializeTest(IO::UART& uart) {
    BMS::BQSetting setting(BMS::BQSetting::BQSettingType::RAM, 4, 0x1122, 0x12345678);
    uint8_t expectedArray[] = { 0x12, 0x22, 0x11, 0x78, 0x56, 0x34, 0x12 };

    uint8_t actualArray[BMS::BQSetting::ARRAY_SIZE];
    setting.toArray(actualArray);


    if (!arraysEqual(expectedArray, actualArray, BMS::BQSetting::ARRAY_SIZE)) {
        uart.printf("Serialization FAILED, expected ");
        printArray(uart, expectedArray, BMS::BQSetting::ARRAY_SIZE);
        uart.printf(" got ");
        printArray(uart, actualArray, BMS::BQSetting::ARRAY_SIZE);
        uart.printf("\r\n");
        return;
    }
    uart.printf("Successful Serialization\r\n");
}

/**
 * Third test, ensure the settings can be serialized and deserialized back
 * and forth.
 */
void serializeDeserializeTest(IO::UART& uart) {
    BMS::BQSetting original(BMS::BQSetting::BQSettingType::SUBCOMMAND, 4, 0x2345, 0x45678901);

    uint8_t serializedArray[BMS::BQSetting::ARRAY_SIZE];
    original.toArray(serializedArray);

    BMS::BQSetting output;
    output.fromArray(serializedArray);

    // Ensure all the values are identical
    if (original.getSettingType() != output.getSettingType()) {
        uart.printf("Ser/Des FAILED, expected setting type %u, got %u\n",
            original.getSettingType(), output.getSettingType());
        return;
    }

    if (original.getAddress() != output.getAddress()) {
        uart.printf("Ser/Des FAILED, expected address 0x%04x, got 0x%04x\r\n",
            original.getAddress(), output.getAddress());
        return;
    }

    if (original.getNumBytes() != output.getNumBytes()) {
        uart.printf("Ser/Des FAILED, expected number of bytes %u, got %u\r\n",
            original.getNumBytes(), output.getNumBytes());
        return;
    }

    if (original.getData() != output.getData()) {
        uart.printf("Ser/Des FAILED, expected data 0x%08x, got 0x%08x\r\n",
            original.getData(), output.getData());
        return;
    }

    uart.printf("Successful Serialization and Deserialization\r\n");
}

int main() {
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);

    uart.printf("\r\n\r\nBQ SETTING TEST\r\n");

    deserializeTest(uart);
    serializeTest(uart);
    serializeDeserializeTest(uart);
}
