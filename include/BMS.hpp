#pragma once

#include <cstdint>

#include <Canopen/co_core.h>

#include <BQSettingStorage.hpp>
#include <EVT/dev/IWDG.hpp>
#include <EVT/io/pin.hpp>
#include <dev/Interlock.hpp>
#include <dev/ResetHandler.hpp>
#include <dev/SystemDetect.hpp>
#include <dev/ThermistorMux.hpp>

#define BQ_COMM_ERROR 0x01
#define BQ_ALARM_ERROR 0x02
#define OVER_TEMP_ERROR 0x04

#define NUM_THERMISTORS 6

namespace IO = EVT::core::IO;

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
        START = 0,
        /// When the BMS fails startup sequence
        INITIALIZATION_ERROR = 1,
        /// When the system is waiting for settings to be sent to the BMS
        FACTORY_INIT = 2,
        /// When the BMS is actively sending settings over to the BQ
        TRANSFER_SETTINGS = 3,
        /// When the BMS is ready for charging / discharging
        SYSTEM_READY = 4,
        /// When the system is running in a low power mode
        DEEP_SLEEP = 5,
        /// When a fault is detected during normal operation
        UNSAFE_CONDITIONS_ERROR = 6,
        /// When the BMS is on the bike and delivering power
        POWER_DELIVERY = 7,
        /// When the BMS is handling charging the battery pack
        CHARGING = 8
    };

    /** BMS Pinout */
    static constexpr IO::Pin OK_PIN = IO::Pin::PA_6;
    static constexpr IO::Pin ALARM_PIN = IO::Pin::PA_5;
    static constexpr IO::Pin UART_TX_PIN = IO::Pin::PA_9;
    static constexpr IO::Pin UART_RX_PIN = IO::Pin::PA_10;
    static constexpr IO::Pin CAN_TX_PIN = IO::Pin::PA_12;
    static constexpr IO::Pin CAN_RX_PIN = IO::Pin::PA_11;
    static constexpr IO::Pin I2C_SCL_PIN = IO::Pin::PB_6;
    static constexpr IO::Pin I2C_SDA_PIN = IO::Pin::PB_7;
    static constexpr IO::Pin INTERLOCK_PIN = IO::Pin::PA_3;
    static constexpr IO::Pin TEMP_INPUT_PIN = IO::Pin::PA_0;
    static constexpr IO::Pin MUX_S1_PIN = IO::Pin::PA_15;
    static constexpr IO::Pin MUX_S2_PIN = IO::Pin::PB_4;
    static constexpr IO::Pin MUX_S3_PIN = IO::Pin::PA_8;

    /**
     * Make a new instance of the BMS with the given devices
     *
     * @param bqSettingsStorage Object used to manage BQ settings storage
     * @param bq BQ chip instance
     * @param interlock GPIO used to check the interlock status
     * @param alarm GPIO used to check the BQ alarm status
     * @param systemDetect Object used to detect what system the BMS is connected to
     * @param bmsOK GPIO used to output the OK signal from the BMS
     * @param thermMux
     * @param resetHandler
     */
    BMS(BQSettingsStorage& bqSettingsStorage, DEV::BQ76952 bq, DEV::Interlock& interlock,
        IO::GPIO& alarm, DEV::SystemDetect& systemDetect, IO::GPIO& bmsOK,
        DEV::ThermistorMux& thermMux, DEV::ResetHandler& resetHandler, EVT::core::DEV::IWDG& iwdg);

    /**
     * The node ID used to identify the device on the CAN network.
     */
    static constexpr uint8_t NODE_ID = 20;

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
     * Set private variables to values that make CAN testing easy
     */
    void canTest();

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
    static constexpr uint16_t OBJECT_DICTIONARY_SIZE = 149;

    /**
     * The active state of the alarm. When the alarm is in this state,
     * the BQ has detected some critical error
     */
    static constexpr IO::GPIO::State ALARM_ACTIVE_STATE =
        IO::GPIO::State::HIGH;

    /**
     * State for representing the BMS is in an OK state to charge/discharge
     */
    static constexpr IO::GPIO::State BMS_OK =
        IO::GPIO::State::HIGH;

    /**
     * State for representing the BMS is in not in an OK state to charge/discharge
     */
    static constexpr IO::GPIO::State BMS_NOT_OK =
        IO::GPIO::State::LOW;

    /**
     * Number of attempts that will be made to communicate with the BQ
     * before failing
     */
    static constexpr uint8_t MAX_BQ_COMM_ATTEMPTS = 3;

    /**
     * Time in milliseconds between attempting a previously failed operation
     */
    static constexpr uint32_t ERROR_TIME_DELAY = 5000;

    /**
     *
     */
    static constexpr uint8_t MAX_THERM_READ_ATTEMPTS = 3;

    /**
     *
     */
    static constexpr uint8_t MAX_THERM_TEMP = 50;

    /**
     * The interface for storing and retrieving BQ Settings.
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
     * If the alarm pin is in it's active state, should assume it is unsafe
     * to charge/discharge
     */
    IO::GPIO& alarm;

    /**
     * This determines which system the BMS is attached to.
     */
    DEV::SystemDetect& systemDetect;

    /**
     *
     */
    DEV::ResetHandler& resetHandler;

    /**
     * This GPIO is used to represent when the system is ok. When this pin
     * is high, it represents that the BMS is in a state ready to
     * charge or discharge,
     */
    IO::GPIO& bmsOK;

    /**
     *
     */
    DEV::ThermistorMux thermistorMux;

    /**
     * Internal watchdog to detect STM hang
     */
    EVT::core::DEV::IWDG& iwdg;

    /**
     * Boolean flag that represents a state has just changed, this is useful
     * for determining when operations should take place that only take place
     * once per state change.
     */
    bool stateChanged = false;

    /**
     * Utility variable which can be used to count the number of attempts that
     * was made to complete a certain actions.
     *
     * For example, this is used for trying to communicate with the BQ N
     * number of times before failing
     */
    uint8_t numAttemptsMade = 0;

    /**
     *
     */
    uint8_t numThermAttemptsMade = 0;

    /**
     * Keeps track of the last time an attempt was made. This is used in
     * combination with BMS::numAttemptsMade to attempt a task a certain
     * number of times with delay in attempts
     */
    uint32_t lastAttemptTime = 0;

    /**
     *
     */
    uint32_t lastThermAttemptTime = 0;

    /**
     * Represents the total voltage read by the BQ chip. This value is updated
     * by reading the voltage from the BQ chip and is then exposed over
     * CANopen.
     */
    uint32_t totalVoltage = 0;

    /**
     * Represents the total voltage in the battery
     */
    uint16_t batteryVoltage = 0;

    /**
     * Represents the total current through the battery
     */
    int16_t current = 0;

    /**
     * Stores the per-thermistor temperature for the battery pack
     */
    uint8_t thermistorTemperature[NUM_THERMISTORS] = {};

    /**
     *
     */
    PackTempInfo packTempInfo{
        .minPackTemp = 0,
        .minPackTempId = 0,
        .maxPackTemp = 0,
        .maxPackTempId = 0,
    };

    /**
     *
     */
    BqTempInfo bqTempInfo{
        .internalTemp = 0,
        .temp1 = 0,
        .temp2 = 0,
    };

    /**
     * Stores the per-cell voltage for the battery pack. This value is updated
     * by reading the voltage from the BQ chip and is then exposed over
     * CANopen.
     */
    uint16_t cellVoltage[DEV::BQ76952::NUM_CELLS] = {};

    /**
     * Used to store values which the BMS updates.
     * Holds information about the minimum and maximum cell's voltages and Ids.
     */
    cellVoltageInfo voltageInfo{
        .minCellVoltage = 0,
        .minCellVoltageId = 0,
        .maxCellVoltage = 0,
        .maxCellVoltageId = 0,
    };

    /**
     *
     */
    uint8_t bqStatusArr[7] = {};

    /**
     *
     */
    uint8_t errorRegister = 0;

    /**
     *
     */
    uint8_t lastCheckedThermNum = -1;

    /**
     * Handles the start of the state machine logic. This considers the health
     * of the system, and the existence of BQ settings.
     *
     * State: State::START
     */
    void startState();

    /**
     * Handles holding the BMS in the initialization error state.
     *
     * State: State::INITIALIZATION_ERROR
     */
    void initializationErrorState();

    /**
     * Handles the factory init state. This will check for the presence of
     * settings having arrived over CANopen.
     *
     * State: State::FACTORY_INIT
     */
    void factoryInitState();

    /**
     * Handles the state where settings are actively being sent from
     * the BMS to the BQ chip.
     *
     * State: State::TRANSFER_SETTINGS
     */
    void transferSettingsState();

    /**
     * Handles when the system is ready for charging/discharging. Will
     * poll health data and sensor information while waiting for input.
     *
     * State: State::SYSTEM_READY
     */
    void systemReadyState();

    /**
     * Handles when an unsafe condition is detected during normal
     * operations.
     *
     * State: State::UNSAFE_CONDITIONS_ERROR
     */
    void unsafeConditionsError();

    /**
     * Handles when the BMS is actively delivering power to the bike
     * system.
     *
     * State: State::POWER_DELIVERY
     */
    void powerDeliveryState();

    /**
     * Handles when the BMS is handling taking in charge.
     *
     * State: State::CHARGING
     */
    void chargingState();

    /**
     * Check to see if the system is healthy. This involves checking the
     * ALARM pin, other status registers on the BQ, and keeping track of the
     * rest of the system.
     *
     * @return True if the system is healthy, false otherwise
     */
    bool isHealthy();

    /**
     * TODO: Update this documentation
     * Update the local voltage variables with the values from the BQ chip.
     * This will iterate over all the voltage values of interest and update
     * each value. These values will then be able to be read over CANopen.
     *
     * This should be called when in a state where the BQ is ready and able
     * to read voltage. In other states, a call to this function should not
     * be present.
     */
    void updateBQData();

    /**
     *
     */
    void updateThermistorReading();

    /**
     * Will clear the local voltage values (set to 0). This is to represent
     * that the voltage readings are either not accurate or not current. For
     * states where the voltage read from the BQ is not expected to be
     * accurate or stable, this should be called so that the data sent out
     * over CANopen reflect that.
     */
    void clearVoltageReadings();

    /**
     * The object dictionary of the BMS. Includes settings that determine
     * how the BMS functions on the CANopen network as well as the data
     * that is exposed on the network.
     *
     * Array of CANopen objects. +1 for the special "end-of-array" marker
     */
    CO_OBJ_T objectDictionary[OBJECT_DICTIONARY_SIZE + 1] = {
        // Sync ID, defaults to 0x80
        {
            .Key = CO_KEY(0x1005, 0, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x80,
        },
        // Information about the hardware, hard coded sample values for now
        // 1: Vendor ID
        // 2: Product Code
        // 3: Revision Number
        // 4: Serial Number
        {
            .Key = CO_KEY(0x1018, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x10},
        {
            .Key = CO_KEY(0x1018, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x11,
        },
        {
            .Key = CO_KEY(0x1018, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x12,
        },
        {
            .Key = CO_KEY(0x1018, 4, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x13,
        },

        // SDO CAN message IDS.
        // 1: Client -> Server ID, default is 0x600 + NODE_ID
        // 2: Server -> Client ID, default is 0x580 + NODE_ID
        {
            .Key = CO_KEY(0x1200, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x600 + NODE_ID,
        },
        {
            .Key = CO_KEY(0x1200, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0x580 + NODE_ID,
        },

        // TPDO0 settings
        // 0: The TPDO number, default 0
        // 1: The COB-ID used by TPDO0, provided as a function of the TPDO number
        // 2: How the TPO is triggered, default to manual triggering
        // 3: Inhibit time, defaults to 0
        // 5: Timer trigger time in 1ms units, 0 will disable the timer based triggering
        {
            .Key = CO_KEY(0x1800, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0,
        },
        {
            .Key = CO_KEY(0x1800, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(0) + NODE_ID,
        },
        {
            .Key = CO_KEY(0x1800, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },
        {
            .Key = CO_KEY(0x1800, 3, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0,
        },
        {
            .Key = CO_KEY(0x1800, 5, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = CO_TEVENT,
            .Data = (uintptr_t) 1000,
        },

        // TPDO1 settings
        // 0: The TPDO number, default 0
        // 1: The COB-ID used by TPDO1, provided as a function of the TPDO number
        // 2: How the TPO is triggered, default to manual triggering
        // 3: Inhibit time, defaults to 0
        // 5: Timer trigger time in 1ms units, 0 will disable the timer based triggering
        {
            .Key = CO_KEY(0x1801, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 1,
        },
        {
            .Key = CO_KEY(0x1801, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(1) + NODE_ID,
        },
        {
            .Key = CO_KEY(0x1801, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },
        {
            .Key = CO_KEY(0x1801, 3, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0,
        },
        {
            .Key = CO_KEY(0x1801, 5, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = CO_TEVENT,
            .Data = (uintptr_t) 1000,
        },

        // TPDO2 settings
        // 0: The TPDO number, default 0
        // 1: The COB-ID used by TPDO2, provided as a function of the TPDO number
        // 2: How the TPO is triggered, default to manual triggering
        // 3: Inhibit time, defaults to 0
        // 5: Timer trigger time in 1ms units, 0 will disable the timer based triggering
        {
            .Key = CO_KEY(0x1802, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 2,
        },
        {
            .Key = CO_KEY(0x1802, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(2) + NODE_ID,
        },
        {
            .Key = CO_KEY(0x1802, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },
        {
            .Key = CO_KEY(0x1802, 3, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0,
        },
        {
            .Key = CO_KEY(0x1802, 5, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = CO_TEVENT,
            .Data = (uintptr_t) 1000,
        },

        // TPDO3 settings
        // 0: The TPDO number, default 0
        // 1: The COB-ID used by TPDO3, provided as a function of the TPDO number
        // 2: How the TPO is triggered, default to manual triggering
        // 3: Inhibit time, defaults to 0
        // 5: Timer trigger time in 1ms units, 0 will disable the timer based triggering
        {
            .Key = CO_KEY(0x1803, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 3,
        },
        {
            .Key = CO_KEY(0x1803, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(3) + NODE_ID,
        },
        {
            .Key = CO_KEY(0x1803, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },
        {
            .Key = CO_KEY(0x1803, 3, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0,
        },
        {
            .Key = CO_KEY(0x1803, 5, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = CO_TEVENT,
            .Data = (uintptr_t) 1000,
        },

        // TPDO4 settings
        // 0: The TPDO number, default 0
        // 1: The COB-ID used by TPDO4, provided as a function of the TPDO number
        // 2: How the TPO is triggered, default to manual triggering
        // 3: Inhibit time, defaults to 0
        // 5: Timer trigger time in 1ms units, 0 will disable the timer based triggering
        {
            .Key = CO_KEY(0x1804, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 4,
        },
        {
            .Key = CO_KEY(0x1804, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(0) + NODE_ID + 10,
        },
        {
            .Key = CO_KEY(0x1804, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },
        {
            .Key = CO_KEY(0x1804, 3, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0,
        },
        {
            .Key = CO_KEY(0x1804, 5, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = CO_TEVENT,
            .Data = (uintptr_t) 1000,
        },

        // TPDO5 settings
        // 0: The TPDO number, default 0
        // 1: The COB-ID used by TPDO5, provided as a function of the TPDO number
        // 2: How the TPO is triggered, default to manual triggering
        // 3: Inhibit time, defaults to 0
        // 5: Timer trigger time in 1ms units, 0 will disable the timer based triggering
        {
            .Key = CO_KEY(0x1805, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 5,
        },
        {
            .Key = CO_KEY(0x1805, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(1) + NODE_ID + 10,
        },
        {
            .Key = CO_KEY(0x1805, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },
        {
            .Key = CO_KEY(0x1805, 3, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0,
        },
        {
            .Key = CO_KEY(0x1805, 5, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = CO_TEVENT,
            .Data = (uintptr_t) 1000,
        },

        // TPDO6 settings
        // 0: The TPDO number, default 0
        // 1: The COB-ID used by TPDO6, provided as a function of the TPDO number
        // 2: How the TPO is triggered, default to manual triggering
        // 3: Inhibit time, defaults to 0
        // 5: Timer trigger time in 1ms units, 0 will disable the timer based triggering
        {
            .Key = CO_KEY(0x1806, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 6,
        },
        {
            .Key = CO_KEY(0x1806, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) CO_COBID_TPDO_DEFAULT(2) + NODE_ID + 10,
        },
        {
            .Key = CO_KEY(0x1806, 2, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0xFE,
        },
        {
            .Key = CO_KEY(0x1806, 3, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 0,
        },
        {
            .Key = CO_KEY(0x1806, 5, CO_UNSIGNED16 | CO_OBJ_D__R_),
            .Type = CO_TEVENT,
            .Data = (uintptr_t) 1000,
        },

        // TPDO0 mapping, determines the PDO messages to send when TPDO1 is triggered
        // 0: The number of PDO messages associated with the TPDO
        // 1: Link to the first PDO message
        // n: Link to the nth PDO message
        {
            .Key = CO_KEY(0x1A00, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 5,
        },
        {
            // batteryVoltage
            .Key = CO_KEY(0x1A00, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 1, 16),
        },
        {
            // minCellVoltage
            .Key = CO_KEY(0x1A00, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 2, 16),
        },
        {
            // minCellVoltageID
            .Key = CO_KEY(0x1A00, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 3, 8),
        },
        {
            // maxCellVoltage
            .Key = CO_KEY(0x1A00, 4, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 4, 16),
        },
        {
            // maxCellVoltageID
            .Key = CO_KEY(0x1A00, 5, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 5, 8),
        },

        // TPDO1 mapping, determines the PDO messages to send when TPDO1 is triggered
        // 0: The number of PDO messages associated with the TPDO
        // 1: Link to the first PDO message
        // n: Link to the nth PDO message
        {
            .Key = CO_KEY(0x1A01, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 7,
        },
        {
            // current
            .Key = CO_KEY(0x1A01, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 6, 16),
        },
        {
            // batteryPackMinTemp
            .Key = CO_KEY(0x1A01, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 7, 8),
        },
        {
            // batteryPackMinTempId
            .Key = CO_KEY(0x1A01, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 8, 8),
        },
        {
            // batteryPackMaxTemp
            .Key = CO_KEY(0x1A01, 4, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 9, 8),
        },
        {
            // batteryPackMaxTempId
            .Key = CO_KEY(0x1A01, 5, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 10, 8),
        },
        {
            // bqInternalTemp
            .Key = CO_KEY(0x1A01, 6, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 11, 8),
        },
        {
            // state
            .Key = CO_KEY(0x1A01, 7, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 12, 8),
        },

        // TPDO2 mapping, determines the PDO messages to send when TPDO2 is triggered
        // 0: The number of PDO messages associated with the TPDO
        // 1: Link to the first PDO message
        // n: Link to the nth PDO message
        {
            .Key = CO_KEY(0x1A02, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 8,
        },
        {
            // packTemp[0]
            .Key = CO_KEY(0x1A02, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 13, 8),
        },
        {
            // packTemp[1]
            .Key = CO_KEY(0x1A02, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 14, 8),
        },
        {
            // packTemp[2]
            .Key = CO_KEY(0x1A02, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 15, 8),
        },
        {
            // packTemp[3]
            .Key = CO_KEY(0x1A02, 4, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 16, 8),
        },
        {
            // packTemp[4]
            .Key = CO_KEY(0x1A02, 5, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 17, 8),
        },
        {
            // packTemp[5]
            .Key = CO_KEY(0x1A02, 6, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 18, 8),
        },
        {
            // boardTemp1
            .Key = CO_KEY(0x1A02, 7, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 19, 8),
        },
        {
            // boardTemp2
            .Key = CO_KEY(0x1A02, 8, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 20, 8),
        },

        // TPDO3 mapping, determines the PDO messages to send when TPDO3 is triggered
        // 0: The number of PDO messages associated with the TPDO
        // 1: Link to the first PDO message
        // n: Link to the nth PDO message
        {
            .Key = CO_KEY(0x1A03, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 8,
        },
        {
            // errorRegister
            .Key = CO_KEY(0x1A03, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 21, 8),
        },
        {
            // bqStatusArr[0]
            .Key = CO_KEY(0x1A03, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 22, 8),
        },
        {
            // bqStatusArr[1]
            .Key = CO_KEY(0x1A03, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 23, 8),
        },
        {
            // bqStatusArr[2]
            .Key = CO_KEY(0x1A03, 4, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 24, 8),
        },
        {
            // bqStatusArr[3]
            .Key = CO_KEY(0x1A03, 5, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 25, 8),
        },
        {
            // bqStatusArr[4]
            .Key = CO_KEY(0x1A03, 6, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 26, 8),
        },
        {
            // bqStatusArr[5]
            .Key = CO_KEY(0x1A03, 7, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 27, 8),
        },
        {
            // bqStatusArr[6]
            .Key = CO_KEY(0x1A03, 8, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 28, 8),
        },

        // TPDO4 mapping, determines the PDO messages to send when TPDO4 is triggered
        // 0: The number of PDO messages associated with the TPDO
        // 1: Link to the first PDO message
        // n: Link to the nth PDO message
        {
            .Key = CO_KEY(0x1A04, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 4,
        },
        {
            // cellVoltage[0]
            .Key = CO_KEY(0x1A04, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 29, 16),
        },
        {
            // cellVoltage[1]
            .Key = CO_KEY(0x1A04, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 30, 16),
        },
        {
            // cellVoltage[2]
            .Key = CO_KEY(0x1A04, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 31, 16),
        },
        {
            // cellVoltage[3]
            .Key = CO_KEY(0x1A04, 4, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 32, 16),
        },

        // TPDO5 mapping, determines the PDO messages to send when TPDO5 is triggered
        // 0: The number of PDO messages associated with the TPDO
        // 1: Link to the first PDO message
        // n: Link to the nth PDO message
        {
            .Key = CO_KEY(0x1A05, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 4,
        },
        {
            // cellVoltage[4]
            .Key = CO_KEY(0x1A05, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 33, 16),
        },
        {
            // cellVoltage[5]
            .Key = CO_KEY(0x1A05, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 34, 16),
        },
        {
            // cellVoltage[6]
            .Key = CO_KEY(0x1A05, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 35, 16),
        },
        {
            // cellVoltage[7]
            .Key = CO_KEY(0x1A05, 4, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 36, 16),
        },

        // TPDO6 mapping, determines the PDO messages to send when TPDO6 is triggered
        // 0: The number of PDO messages associated with the TPDO
        // 1: Link to the first PDO message
        // n: Link to the nth PDO message
        {
            .Key = CO_KEY(0x1A06, 0, CO_UNSIGNED8 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = (uintptr_t) 4,
        },
        {
            // cellVoltage[8]
            .Key = CO_KEY(0x1A06, 1, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 37, 16),
        },
        {
            // cellVoltage[9]
            .Key = CO_KEY(0x1A06, 2, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 38, 16),
        },
        {
            // cellVoltage[10]
            .Key = CO_KEY(0x1A06, 3, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 39, 16),
        },
        {
            // cellVoltage[11]
            .Key = CO_KEY(0x1A06, 4, CO_UNSIGNED32 | CO_OBJ_D__R_),
            .Type = nullptr,
            .Data = CO_LINK(0x2100, 40, 16),
        },

        // User defined data, this will be where we put elements that can be
        // accessed via SDO and depending on configuration PDO
        {
            .Key = CO_KEY(0x2100, 1, CO_SIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &batteryVoltage,
        },
        {
            .Key = CO_KEY(0x2100, 2, CO_SIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &voltageInfo.minCellVoltage,
        },
        {
            .Key = CO_KEY(0x2100, 3, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &voltageInfo.minCellVoltageId,
        },
        {
            .Key = CO_KEY(0x2100, 4, CO_SIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &voltageInfo.maxCellVoltage,
        },
        {
            .Key = CO_KEY(0x2100, 5, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &voltageInfo.maxCellVoltageId,
        },
        {
            .Key = CO_KEY(0x2100, 6, CO_SIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &current,
        },
        {
            .Key = CO_KEY(0x2100, 7, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &packTempInfo.minPackTemp,
        },
        {
            .Key = CO_KEY(0x2100, 8, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &packTempInfo.minPackTempId,
        },
        {
            .Key = CO_KEY(0x2100, 9, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &packTempInfo.maxPackTemp,
        },
        {
            .Key = CO_KEY(0x2100, 10, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &packTempInfo.maxPackTempId,
        },
        {
            .Key = CO_KEY(0x2100, 11, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &bqTempInfo.internalTemp,
        },
        {
            .Key = CO_KEY(0x2100, 12, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &state,
        },
        {
            .Key = CO_KEY(0x2100, 13, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &thermistorTemperature[0],
        },
        {
            .Key = CO_KEY(0x2100, 14, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &thermistorTemperature[1],
        },
        {
            .Key = CO_KEY(0x2100, 15, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &thermistorTemperature[2],
        },
        {
            .Key = CO_KEY(0x2100, 16, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &thermistorTemperature[3],
        },
        {
            .Key = CO_KEY(0x2100, 17, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &thermistorTemperature[4],
        },
        {
            .Key = CO_KEY(0x2100, 18, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &thermistorTemperature[5],
        },
        {
            .Key = CO_KEY(0x2100, 19, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &bqTempInfo.temp1,
        },
        {
            .Key = CO_KEY(0x2100, 20, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &bqTempInfo.temp2,
        },
        {
            .Key = CO_KEY(0x2100, 21, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &errorRegister,
        },
        {
            .Key = CO_KEY(0x2100, 22, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &bqStatusArr[0],
        },
        {
            .Key = CO_KEY(0x2100, 23, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &bqStatusArr[1],
        },
        {
            .Key = CO_KEY(0x2100, 24, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &bqStatusArr[2],
        },
        {
            .Key = CO_KEY(0x2100, 25, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &bqStatusArr[3],
        },
        {
            .Key = CO_KEY(0x2100, 26, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &bqStatusArr[4],
        },
        {
            .Key = CO_KEY(0x2100, 27, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &bqStatusArr[5],
        },
        {
            .Key = CO_KEY(0x2100, 28, CO_UNSIGNED8 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &bqStatusArr[6],
        },
        {
            .Key = CO_KEY(0x2100, 29, CO_UNSIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &cellVoltage[0],
        },
        {
            .Key = CO_KEY(0x2100, 30, CO_UNSIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &cellVoltage[1],
        },
        {
            .Key = CO_KEY(0x2100, 31, CO_UNSIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &cellVoltage[2],
        },
        {
            .Key = CO_KEY(0x2100, 32, CO_UNSIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &cellVoltage[3],
        },
        {
            .Key = CO_KEY(0x2100, 33, CO_UNSIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &cellVoltage[4],
        },
        {
            .Key = CO_KEY(0x2100, 34, CO_UNSIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &cellVoltage[5],
        },
        {
            .Key = CO_KEY(0x2100, 35, CO_UNSIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &cellVoltage[6],
        },
        {
            .Key = CO_KEY(0x2100, 36, CO_UNSIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &cellVoltage[7],
        },
        {
            .Key = CO_KEY(0x2100, 37, CO_UNSIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &cellVoltage[8],
        },
        {
            .Key = CO_KEY(0x2100, 38, CO_UNSIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &cellVoltage[9],
        },
        {
            .Key = CO_KEY(0x2100, 39, CO_UNSIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &cellVoltage[10],
        },
        {
            .Key = CO_KEY(0x2100, 40, CO_UNSIGNED16 | CO_OBJ___PR_),
            .Type = nullptr,
            .Data = (uintptr_t) &cellVoltage[11],
        },

        /// Expose information on the balancing of the target cells. Per
        /// cell ability to poll if the cell is balancing and write out
        /// balancing control.
        {
            .Key = CO_KEY(0x2103, 1, CO_UNSIGNED8 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 1,
        },
        {
            .Key = CO_KEY(0x2103, 2, CO_UNSIGNED8 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 2,
        },
        {
            .Key = CO_KEY(0x2103, 3, CO_UNSIGNED8 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 3,
        },
        {
            .Key = CO_KEY(0x2103, 4, CO_UNSIGNED8 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 4,
        },

        {
            .Key = CO_KEY(0x2103, 5, CO_UNSIGNED8 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 5,
        },
        {
            .Key = CO_KEY(0x2103, 6, CO_UNSIGNED8 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 6,
        },
        {
            .Key = CO_KEY(0x2103, 7, CO_UNSIGNED8 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 7,
        },
        {
            .Key = CO_KEY(0x2103, 8, CO_UNSIGNED8 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 8,
        },
        {
            .Key = CO_KEY(0x2103, 9, CO_UNSIGNED8 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 9,
        },
        {
            .Key = CO_KEY(0x2103, 10, CO_UNSIGNED8 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 10,
        },
        {
            .Key = CO_KEY(0x2103, 11, CO_UNSIGNED8 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 11,
        },
        {
            .Key = CO_KEY(0x2103, 12, CO_UNSIGNED8 | CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 12,
        },

        // End of dictionary marker
        CO_OBJ_DIR_ENDMARK,
    };
};

}// namespace BMS
