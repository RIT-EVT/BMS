#pragma once

#include <EVT/io/GPIO.hpp>

namespace BMS::DEV {

/**
 * Represents the interlock which detects if the battery is has a cable
 * connected to it.
 */
class Interlock {
public:
    /**
     * Create an interlock which will detect the presence of a cable via
     * the provided GPIO.
     */
    Interlock(EVT::core::IO::GPIO& gpio);

    /**
     * See if a cable is detected in the interlock
     */
    bool isDetected();

private:
    /** Active high state of the detect GPIO */
    static constexpr EVT::core::IO::GPIO::State ACTIVE_STATE = EVT::core::IO::GPIO::State::HIGH;

    /** GPIO which is used to read the detect state */
    EVT::core::IO::GPIO& gpio;
};

}// namespace BMS::DEV
