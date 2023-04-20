#pragma once

#include <EVT/io/GPIO.hpp>
#include <EVT/dev/Thermistor.hpp>
#include <EVT/utils/time.hpp>

namespace IO = EVT::core::IO;
namespace time = EVT::core::time;

namespace BMS::DEV {

/**
 *
 */
class ThermistorMux {
public:
    /**
     * The number of thermistors connected to the mux
     */
    static constexpr uint8_t NUM_THERMISTORS = 6;

    /**
     *
     *
     * @param muxSelectArr
     * @param adc
     */
    ThermistorMux(IO::GPIO* muxSelectArr[3], IO::ADC& adc);

    /**
     *
     *
     * @param thermNum
     * @return
     */
    uint16_t getTemp(uint8_t thermNum);

private:
    /** Array of MUX select pins */
    IO::GPIO* muxSelectArr[3];
    /** Thermistor instance to read the temperatures with */
    EVT::core::DEV::Thermistor therm;

    /**
     * T(x) = 0.00000375688x^2 + 0.0121347x - 15.9911
     * Returns value in Celsius
     *
     * @param adcCounts
     * @return
     */
    static uint32_t convert(uint32_t adcCounts) {
        uint16_t temp;

        temp = (((uint64_t) adcCounts * adcCounts) * 375688 + ((uint64_t)adcCounts) * 1213470000 - 1599110000000) / 100000000000;

        return temp;
    }
};

}// namespace BMS::DEV

