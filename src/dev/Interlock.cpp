#include <dev/Interlock.hpp>

namespace BMS::DEV {

Interlock::Interlock(EVT::core::IO::GPIO& gpio) : gpio(gpio) {}

bool Interlock::isDetected() {
    // TODO: Use debounce for handling potential noise based issues
    return gpio.readPin() == ACTIVE_STATE;
}

}// namespace BMS::DEV
