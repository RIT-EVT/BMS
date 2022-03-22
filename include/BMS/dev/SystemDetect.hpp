#pragma once

#include <cstdint>

namespace BMS::DEV {

/**
 * Device which has the ability to determine if the BMS system is
 * connected to the charger or the bike system.
 *
 * This is handled by checking for a CANopen heart beat for specific
 * devices (pre-charge board for the bike, charge controller for the
 * charging). The device also has the ability to check how long since
 * the last heart beat has been detected.
 *
 * The main way this device is used is with a CAN interrupt handler.
 * Essentially, this device should be passed into the CAN interrupt
 * handler and given the ability to check for the specific heart beat
 * values.
 */
class SystemDetect {
public:

    /**
     * The different systems that could be detected. If no heart beat
     * has been processed within a given timeout, the system is left as
     * unknown.
     */
    enum class System {
        BIKE = 0,
        CHARGER = 1,
        UNKNOWN = 3
    };

    /**
     * Create the system detect device which will work to indetify the
     * provided heart beat CANopen IDs.
     *
     * @param[in] bikeHeartBeat The heart beat CANopen ID associated with the
     *                          bike.
     * @param[in] chargeHeatBeat The heat beat CANopen ID associated with the
     *                           charger
     * @param[in] timeout The timeout which represents time between getting
     *                    heart beat values where the device still recognizes
     *                    the system it is attached to. If the heart beat is
     *                    not received within this timeout, device assumes it
     *                    does not know what the system is attached to. This
     * heart beat has been detected
     *                    is in milliseconds.
     */
    SystemDetect(uint32_t bikeHeatBeat, uint32_t chargeHeatBeat, uint32_t timeout);

    /**
     * Check the given CAN ID to see if it represents a system detect
     * heat beat.
     *
     * @param[in] heartBeatID The ID to potentially identify as a system detect
     */
    void processHeartBeat(uint32_t heartBeatID);

    /**
     * Get the currently detected system, could be unknown.
     *
     * @return The current identified system
     */
    System getIdentifiedSystem();

private:
    /** The CANopen ID associated with the bike */
    uint32_t bikeHeartBeat;
    /** The CANopen ID associated with the charger */
    uint32_t chargeHeartBeat;
    /** Timeout when the device does not recongize what it is attached to */
    uint32_t timeout;
    /** Represents the time since last read */
    uint32_t lastRead = 0;
    /** The currently identified system */
    System identifiedSystem;

};

}
