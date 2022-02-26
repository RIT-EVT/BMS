#include <BMS/BQSetting.hpp>
#include <BMS/BMSLogger.hpp>

namespace BMS {

BQSetting::BQSetting(BQSettingType settingType, uint8_t numBytes, uint16_t address, uint32_t data) :
    settingType(settingType), address(address), data(data), numBytes(numBytes) {

}

BQSetting::BQSetting() {
}

void BQSetting::fromArray(uint8_t buffer[ARRAY_SIZE]) {
    settingType = static_cast<BQSettingType>(buffer[0] & 0x3);
    numBytes = (buffer[0] >> 2) & 0x7;

    address = (static_cast<uint16_t>(buffer[2]) << 8) | buffer[1];

    data = (static_cast<uint32_t>(buffer[6]) << 24)
           | (static_cast<uint32_t>(buffer[5]) << 16)
           | (static_cast<uint32_t>(buffer[4]) << 8)
           | static_cast<uint32_t>(buffer[3]);

    LOGGER.log(BMSLogger::LogLevel::DEBUG,
        "Command Type: %u, Address: 0x%04X, Num Bytes: %u, DataL 0x%08X",
        settingType, address, numBytes, data);
}

void BQSetting::toArray(uint8_t buffer[ARRAY_SIZE]) {
    // Command byte
    uint8_t commandByte = static_cast<uint8_t>(settingType) | (numBytes << 2);
    buffer[0] = commandByte;

    // Address
    buffer[1] = address & 0xFF;
    buffer[2] = address >> 8;

    // Data
    buffer[3] = data & 0xFF;
    buffer[4] = (data >> 8) & 0xFF;
    buffer[5] = (data >> 16) & 0xFF;
    buffer[6] = (data >> 24) & 0xFF;
}

BQSetting::BQSettingType BQSetting::getSettingType() {
    return settingType;
}

uint16_t BQSetting::getAddress() {
    return address;
}

uint32_t BQSetting::getData() {
    switch(numBytes) {
        case 1:
            return data & 0xFF;
        case 2:
            return data & 0xFFFF;
        case 3:
            return data & 0xFFFFFF;
        case 4:
            return data & 0xFFFFFFFF;
    }
    return data;
}

uint8_t BQSetting::getNumBytes() {
    return numBytes;
}

}  // namespace BMS
