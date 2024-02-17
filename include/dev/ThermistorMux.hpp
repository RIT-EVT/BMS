#pragma once

#include <EVT/dev/Thermistor.hpp>
#include <EVT/io/GPIO.hpp>
#include <EVT/utils/time.hpp>

namespace IO = EVT::core::IO;
namespace time = EVT::core::time;

namespace BMS::DEV {

/**
 * Multiplexer connected to thermistors
 */
class ThermistorMux {
public:
    /**
     * Create a ThermistorMux instance
     *
     * @param[in] muxSelectArr MUX select pins
     * @param[in] adc ADC instance to read
     */
    ThermistorMux(IO::GPIO* muxSelectArr[3], IO::ADC& adc);

    /**
     * Get temperature from one thermistor
     *
     * @param[in] thermNum Number of thermistor to read
     * @return Thermistor temperature
     */
    uint16_t getTemp(uint8_t thermNum);

private:
    /** Array of MUX select pins */
    IO::GPIO* muxSelectArr[3];
    /** Thermistor instance to read the temperatures with */
    EVT::core::DEV::Thermistor therm;

    /**
     * Conversion equation from ADC counts to temperature in Celsius
     *
     * T(x) = 0.00000375688x^2 + 0.0121347x - 15.9911
     * Returns value in Celsius
     *
     * @param adcCounts ADC reading to convert
     * @return Thermistor temperature in Celsius
     */
    static uint32_t convert(uint32_t adcCounts) {
        uint16_t temp;

        temp = (((uint64_t) adcCounts * adcCounts) * 375688 + ((uint64_t) adcCounts) * 1213470000 - 1599110000000) / 100000000000;

        return temp;
    }
};

}// namespace BMS::DEV
