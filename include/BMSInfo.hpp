#pragma once

#include <cstdint>

namespace BMS {

/**
 * Holds closely-related information about battery voltages to pass into methods more concisely
 *
 * @var minCellVoltage Minimum cell voltage
 * @var minCellVoltageId ID of the cell with the minimum voltage
 * @var maxCellVoltage Maximum cell voltage
 * @var maxCellVoltageId ID of the cell with the maximum voltage
 */
struct CellVoltageInfo {
    int16_t minCellVoltage;
    uint8_t minCellVoltageId;
    int16_t maxCellVoltage;
    uint8_t maxCellVoltageId;
};

/**
 * Holds closely-related information about pack temperature to pass into methods more concisely
 *
 * @var minPackTemp Minimum sensor temperature
 * @var minPackTempId ID of the sensor with the minimum temperature
 * @var maxPackTemp Maximum sensor temperature
 * @var maxPackTempId ID of the sensor with the maximum temperature
 */
struct PackTempInfo {
    uint8_t minPackTemp;
    uint8_t minPackTempId;
    uint8_t maxPackTemp;
    uint8_t maxPackTempId;
};

/**
 * Holds closely-related information about BQ-measured temperatures to pass into methods more
 * concisely
 *
 * @var internalTemp Internal temperature of the BQ chip
 * @var temp1 First on-board temperature measured by the BQ chip
 * @var temp2 Second on-board temperature measured by the BQ chip
 */
struct BqTempInfo {
    uint8_t internalTemp;
    uint8_t temp1;
    uint8_t temp2;
};

}// namespace BMS
