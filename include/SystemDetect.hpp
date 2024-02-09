#pragma once

#include <cstdint>

namespace BMS {

/**
 * Device which has the ability to determine if the BMS system is
 * connected to the charger or the bike system.
 *
 * This is handled by checking for a CANopen heartbeat for specific
 * devices (pre-charge board for the bike, charge controller for the
 * charging). The device also has the ability to check how long since
 * the last heartbeat has been detected.
 *
 * The main way this device is used is with a CAN interrupt handler.
 * Essentially, this device should be passed into the CAN interrupt
 * handler and given the ability to check for the specific heartbeat
 * values.
 */
class SystemDetect {
public:
    /**
     * The different systems that could be detected. If no heartbeat
     * has been processed within a given timeout, the system is left as
     * unknown.
     */
    enum class System {
        BIKE = 0,
        CHARGER = 1,
        UNKNOWN = 3
    };

    /**
     * Create the system detect device which will work to identify the
     * provided  beat CANopen IDs.
     *
     * @param[in] bikeHeartbeat The heartbeat CANopen ID associated with the
     *                          bike.
     * @param[in] chargeHeartbeat The heartbeat CANopen ID associated with the
     *                           charger
     * @param[in] timeout The timeout which represents time between getting
     *                    heartbeat values where the device still recognizes
     *                    the system it is attached to. If the heartbeat is
     *                    not received within this timeout, device assumes it
     *                    does not know what the system is attached to. This
     *                    heartbeat has been detected
     *                    is in milliseconds.
     */
    SystemDetect(uint32_t bikeHeartbeat, uint32_t chargeHeartbeat, uint32_t timeout);

    /**
     * Check the given CAN ID to see if it represents a system detect
     * heartbeat.
     *
     * @param[in] heartbeatID The ID to potentially identify as a system detect
     */
    void processHeartbeat(uint32_t heartbeatID);

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
    uint32_t chargeHeartbeat;
    /** Timeout when the device does not recognize what it is attached to */
    uint32_t timeout;
    /** Represents the time since last read */
    uint32_t lastRead = 0;
    /** The currently identified system */
    System identifiedSystem;
};

}// namespace BMS
