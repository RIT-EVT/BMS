#pragma once
#include <stdint.h>

struct cellVoltageInfo {
    int16_t minVoltage;
    uint8_t minCellVoltageID;
    int16_t maxVoltage;
    uint8_t maxCellVoltageId;
};