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
