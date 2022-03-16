#include <BMS/dev/Interlock.hpp>

namespace BMS::DEV {

Interlock::Interlock(EVT::core::IO::GPIO& gpio) : gpio(gpio) {}

bool Interlock::isDetected() {
    return gpio.readPin() == ACTIVE_STATE;
}

}// namespace BMS::DEV
