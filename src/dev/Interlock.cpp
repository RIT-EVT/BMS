#include <dev/Interlock.hpp>

namespace BMS::dev {

Interlock::Interlock(core::io::GPIO& gpio) : gpio(gpio) {}

bool Interlock::isDetected() {
    // TODO: Use debounce for handling potential noise based issues
    return gpio.readPin() == ACTIVE_STATE;
}

}// namespace BMS::dev
