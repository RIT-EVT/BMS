#pragma once

#include <cstdint>

/*
 * Holds closely-related information about battery voltages to pass into methods more concisely
 */
struct cellVoltageInfo {
    int16_t minCellVoltage;
    uint8_t minCellVoltageId;
    int16_t maxCellVoltage;
    uint8_t maxCellVoltageId;
};

struct PackTempInfo {
    uint8_t minPackTemp;
    uint8_t minPackTempId;
    uint8_t maxPackTemp;
    uint8_t maxPackTempId;
};

struct BqTempInfo {
    uint8_t internalTemp;
    uint8_t temp1;
    uint8_t temp2;
};
