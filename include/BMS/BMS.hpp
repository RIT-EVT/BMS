#pragma once

#include <BMS/BQSettingStorage.hpp>
#include <BMS/dev/Interlock.hpp>
#include <Canopen/co_core.h>
#include <stdint.h>

namespace BMS {

/**
 * Interface to the BMS board. Includes the CANopen object dictionary that
 * defined the features that are exposed by the BMS on the CANopen network.
 */
class BMS {
public:
    /**
     * Represents the different states the BMS can be in. At any point,
     * it will be in one of these states
     */
    enum class State {
        /// When the BMS is powered on
        START,
        /// When the BMS fails startup sequence
        INITIALIZATION_ERROR,
        /// When the system is waiting for settings to be sent to the BMS
        FACTORY_INIT,
        /// When the BMS is actively sending settings over to the BQ
        TRANSFER_SETTINGS,
        /// When the BMS is ready for charging / discharging
        SYSTEM_READY,
        /// When the system is running in a low power mode
        DEEP_SLEEP,
        /// When a fault is detected during normal operation
        UNSAFE_CONDITIONS_ERROR,
        /// When the BMS is on the bike and delivering power
        POWER_DELIVERY,
        /// When the BMS is handling charging the battery pack
        CHARGING
    };

    BMS(BQSettingsStorage& bqSettingsStorage, DEV::BQ76952 bq, DEV::Interlock& interlock, EVT::core::IO::GPIO& alarm);

    /**
     * The node ID used to identify the device on the CAN network.
     */
    static constexpr uint8_t NODE_ID = 0x05;

    /**
     * Get a pointer to the start of the CANopen object dictionary.
     *
     * @return Pointer to the start of the CANopen object dictionary.
     */
    CO_OBJ_T* getObjectDictionary();

    /**
     * Get the number of elements in the object dictionary.
     *
     * @return The number of elements in the object dictionary
     */
    uint16_t getObjectDictionarySize();

    /**
     * Handle running the core logic of the BMS. This involves
     * 1. Checking for state machine related updates
     * 2. Polling sensor diagnostic sensor information
     * 3. Responding to error conditions
     */
    void process();

private:
    /**
     * Have to know the size of the object dictionary for initialization
     * process.
     */
    static constexpr uint16_t OBJECT_DIRECTIONARY_SIZE = 17;

    /**
     * The active state of the alarm. When the alarm is in this state,
     * the BQ has detected some critical error
     */
    static constexpr EVT::core::IO::GPIO::State ALARM_ACTIVE_STATE =
        EVT::core::IO::GPIO::State::HIGH;

    /**
     * The interface for storaging and retrieving BQ Settings.
     */
    BQSettingsStorage& bqSettingsStorage;

    /**
     * Interface to the BQ chip.
     */
    DEV::BQ76952 bq;

    /**
     * The current state of the BMS
     */
    State state;

    /**
     * The interlock which is used to detect a cable plugged in
     */
    DEV::Interlock& interlock;

    /**
     * This GPIO is connected to the ALARM pin of the BQ. The BQ can be
     * configured to toggle the ALARM pin based on certain safety parameters.
     * If the alarm pin is in it's active state, should assum it is unsafe
     * to charge/discharge
     */
    EVT::core::IO::GPIO& alarm;

    /**
     * Handles the start of the state machine logic. This considers the health
     * of the system, and the existance of BQ settings.
     *
     * State: State::START
     *
     * Output:
     *
     * State Transitions:
     */
    void startState();

    /**
     * Handles holding the BMS in the intialization error state.
     *
     * State: State::INITIALIZATION_ERROR
     *
     * Output:
     *
     * State Transitions:
     *
     */
    void initializationErrorState();

    /**
     * Handles the factory init state. This will check for the presence of
     * settings having arrived over CANopen.
     *
     * State: State::FACTORY_INIT
     *
     * Output:
     *
     * State Transitions:
     */
    void factoryInitState();

    /**
     * Handles the state where settings are activly being sent from
     * the BMS to the BQ chip.
     *
     * State: State::TRANSFER_SETTINGS
     *
     * Output:
     *
     * State Transitions:
     */
    void transferSettingsState();

    /**
     * Handles when the system is ready for charging/discharging. Will
     * poll health data and sensor information while waiting for input.
     *
     * State: State::SYSTEM_READY
     *
     * Output:
     *
     * State Transitions:
     */
    void systemReadyState();

    /**
     * Handles when an unsafe condition is detected during normal
     * operations.
     *
     * State: State::UNSAFE_CONDITIONS_ERROR
     *
     * Output:
     *
     * State Transitions:
     */
    void unsafeConditionsError();

    /**
     * Handles when the BMS is actively delivering power to the bike
     * system.
     *
     * State: State::POWER_DELIVERY
     *
     * Output:
     *
     * State Transitions:
     */
    void powerDeliveryState();

    /**
     * Handles when the BMS is handling taking in charge.
     *
     * State: State::CHARGING
     *
     * Output:
     *
     * State Transitions:
     */
    void chargingState();

    /**
     * The object dictionary of the BMS. Includes settings that determine
     * how the BMS functions on the CANopen network as well as the data
     * that is exposed on the network.
     *
     * Array of CANopen objects. +1 for the special "end-of-array" marker
     */
    CO_OBJ_T objectDictionary[OBJECT_DIRECTIONARY_SIZE + 1] = {
        // Sync ID, defaults to 0x80
        {CO_KEY(0x1005, 0, CO_UNSIGNED32 | CO_OBJ_D__R_), 0, (uintptr_t) 0x80},

        // Information about the hardware, hard coded sample values for now
        // 1: Vendor ID
        // 2: Product Code
        // 3: Revision Number
        // 4: Serial Number
        {
            .Key = CO_KEY(0x1018, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t) 0x10,
        },
        {
            .Key = CO_KEY(0x1018, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t) 0x11,
        },
        {
            .Key = CO_KEY(0x1018, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t) 0x12,
        },
        {
            .Key = CO_KEY(0x1018, 4, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t) 0x13,
        },

        // SDO CAN message IDS.
        // 1: Client -> Server ID, default is 0x600 + NODE_ID
        // 2: Server -> Client ID, default is 0x580 + NODE_ID
        {
            .Key = CO_KEY(0x1200, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t) 0x600 + NODE_ID,
        },
        {
            .Key = CO_KEY(0x1200, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t) 0x580 + NODE_ID,
        },

        // TPDO0 settings
        // 0: The TPDO number, default 0
        // 1: The COB-ID used by TPDO0, provided as a function of the TPDO
        //    number
        // 2: How the TPO is triggered, default to manual triggering
        // 3: Inhibit time, defaults to 0
        // 5: Timer trigger time in 1ms units, 0 will disable the timer based
        //    triggering
        {
            .Key = CO_KEY(0x1800, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t) 5,
        },
        {
            .Key = CO_KEY(0x1800, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t) 0x40000180 + NODE_ID,
        },
        {
            .Key = CO_KEY(0x1800, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t) 0xFE,
        },
        {
            .Key = CO_KEY(0x1800, 3, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t) 0,
        },
        {
            .Key = CO_KEY(0x1800, 5, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = CO_TEVENT,
            .Data = (uintptr_t) 2000,
        },

        // TPDO0 mapping, determins the PDO messages to send when TPDO1 is triggered
        // 0: The number of PDO message associated with the TPDO
        // 1: Link to the first PDO message
        // n: Link to the nth PDO message
        {
            .Key = CO_KEY(0x1A00, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t) 1,
        },
        {
            .Key = CO_KEY(0x1A00, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = 0,
            .Data = CO_LINK(0x2100, 0, 8),// Link to sample data position in dictionary
        },

        // BQ Settings exposure over CANopen. The number of settings can be
        // read and written to over CANopen and the array of settings
        // themselves can be read and written to. The BQ settings storage
        // logic is controlled by the custom BQSettingStorage object
        {
            // The number of settings stored
            .Key = CO_KEY(0x2100, 0, CO_UNSIGNED16 | CO_OBJ___PRW),
            .Type = 0,
            .Data = (uintptr_t) &bqSettingsStorage.numSettings,
        },
        {
            // The BQ settings themselves
            .Key = CO_KEY(0x2100, 1, CO_UNSIGNED32 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bqSettingsStorage.canOpenInterface),
            .Data = (uintptr_t) &bqSettingsStorage.canOpenInterface,
        },

        // Voltage values, as read from the BQ chip. The total voltage will
        // periodically be broadcasted as a PDO. The individual series cell
        // voltages will not be broadcasted via PDO, but will still be
        // accessible over SDO.
        {
            .Key = CO_KEY(0x2101, 0, CO_UNSIGNED32 | CO_OBJ___PR_),
            .Type = 0,
            .Data = (uintptr_t) bq.totalVoltage,
        },

        // End of dictionary marker
        CO_OBJ_DIR_ENDMARK,
    };
};
}// namespace BMS
