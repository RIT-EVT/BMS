#pragma once

#include <core/io/GPIO.hpp>

namespace BMS::dev {

/**
 * Represents the interlock which detects if the battery is has a cable
 * connected to it.
 */
class Interlock {
public:
    /**
     * Create an interlock which will detect the presence of a cable via
     * the provided GPIO.
     *
     * @pre The GPIO is set as an input
     */
    explicit Interlock(core::io::GPIO& gpio);

    /**
     * See if a cable is detected in the interlock
     */
    bool isDetected();

private:
    /** Active high state of the detect GPIO */
    static constexpr core::io::GPIO::State ACTIVE_STATE = core::io::GPIO::State::HIGH;

    /** GPIO which is used to read the detect state */
    core::io::GPIO& gpio;
};

}// namespace BMS::dev
