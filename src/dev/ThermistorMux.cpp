#include <dev/ThermistorMux.hpp>

namespace BMS::DEV {

ThermistorMux::ThermistorMux(IO::GPIO** muxSelectArr, IO::ADC& adc) : muxSelectArr{
    muxSelectArr[0], muxSelectArr[1], muxSelectArr[2]},
                                                                      therm(EVT::core::DEV::Thermistor(adc, convert)) {}

uint16_t ThermistorMux::getTemp(uint8_t thermNum) {
    for (uint8_t i = 0; i < 3; i++) {
        muxSelectArr[i]->writePin(thermNum & 1 << i ? IO::GPIO::State::HIGH : IO::GPIO::State::LOW);
    }

    time::wait(40);
    return therm.getTempCelcius();
}

}// namespace BMS::DEV
