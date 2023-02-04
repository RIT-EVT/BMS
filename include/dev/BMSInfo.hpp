#pragma once
#include <stdint.h>
/*
 * Holds the declarations of any information which the BMS updates.
 */

//A struct used by the updateVoltageReadings() to consolidate and reduce the number of arguments which are passed as parameters.
struct cellVoltageInfo {
    int16_t minCellVoltage;
    uint8_t minCellVoltageID;
    int16_t maxCellVoltage;
    uint8_t maxCellVoltageId;
};
