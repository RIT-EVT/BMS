#pragma once

#include <cstdint>

#include <co_core.h>
#include <EVT/io/CANDevice.hpp>
#include <EVT/io/CANOpenMacros.hpp>

#include <BQSettingStorage.hpp>
#include <EVT/dev/IWDG.hpp>
#include <EVT/io/pin.hpp>
#include <ResetHandler.hpp>
#include <SystemDetect.hpp>
#include <dev/Interlock.hpp>
#include <dev/ThermistorMux.hpp>

namespace IO = EVT::core::IO;

namespace BMS {

/**
 * Interface to the BMS board. Includes the CANopen object dictionary that
 * defines the features that are exposed by the BMS on the CANopen network.
 */
class BMS : public CANDevice {
public:
    /**
     * Represents the different states the BMS can be in
     */
    enum class State {
        /** When the BMS is powered on */
        START = 0,
        /** When the BMS fails startup sequence */
        INITIALIZATION_ERROR = 1,
        /** When the system is waiting for settings to be sent to the BMS */
        FACTORY_INIT = 2,
        /** When the BMS is actively sending settings over to the BQ */
        TRANSFER_SETTINGS = 3,
        /** When the BMS is ready for charging / discharging */
        SYSTEM_READY = 4,
        /** When the system is running in a low power mode */
        DEEP_SLEEP = 5,
        /** When a fault is detected during normal operation */
        UNSAFE_CONDITIONS_ERROR = 6,
        /** When the BMS is on the bike and delivering power */
        POWER_DELIVERY = 7,
        /** When the BMS is handling charging the battery pack */
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

    /** Error values */
    static constexpr uint8_t BQ_COMM_ERROR = 0x01;
    static constexpr uint8_t BQ_ALARM_ERROR = 0x02;
    static constexpr uint8_t OVER_TEMP_ERROR = 0x04;

    /**
     * Make a new instance of the BMS with the given devices
     *
     * @param bqSettingsStorage Object used to manage BQ settings storage
     * @param bq BQ chip instance
     * @param interlock GPIO used to check the interlock status
     * @param alarm GPIO used to check the BQ alarm status
     * @param systemDetect Object used to detect what system the BMS is connected to
     * @param bmsOK GPIO used to output the OK signal from the BMS
     * @param thermMux MUX for pack thermistors
     * @param resetHandler Handler for reset messages
     */
    BMS(BQSettingsStorage& bqSettingsStorage, DEV::BQ76952 bq, DEV::Interlock& interlock,
        IO::GPIO& alarm, SystemDetect& systemDetect, IO::GPIO& bmsOK,
        DEV::ThermistorMux& thermMux, ResetHandler& resetHandler, EVT::core::DEV::IWDG& iwdg);

    /**
     * The node ID used to identify the device on the CAN network.
     */
    static constexpr uint8_t NODE_ID = 20;

    /**
     * Get a pointer to the start of the CANopen object dictionary.
     *
     * @return Pointer to the start of the CANopen object dictionary.
     */
    CO_OBJ_T* getObjectDictionary() override;

    /**
     * Get the number of elements in the object dictionary.
     *
     * @return The number of elements in the object dictionary
     */
    uint8_t getNumElements() override;

    /**
     * Get the device's node ID
     *
     * @return the node ID of the CAN device
     */
    uint8_t getNodeID() override;

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
    static constexpr uint16_t OBJECT_DICTIONARY_SIZE = 154;

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
     * Maximum number of attempts to read a safe thermistor temperature before
     * throwing an error
     */
    static constexpr uint8_t MAX_THERM_READ_ATTEMPTS = 3;

    /**
     * Maximum thermistor temperature considered safe
     */
    static constexpr uint8_t MAX_THERM_TEMP = 50;

    /**
     * Number of thermistors in the pack
     */
    static constexpr uint8_t NUM_THERMISTORS = 6;

    /**
     * The interface for storing and retrieving BQ Settings
     */
    BQSettingsStorage& bqSettingsStorage;

    /**
     * Interface to the BQ chip
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
     * This GPIO is connected to the ALARM pin of the BQ
     *
     * The BQ can be configured to toggle the ALARM pin based on certain safety
     * parameters. If the alarm pin is in it's active state, should assume it is
     * unsafe to charge/discharge.
     */
    IO::GPIO& alarm;

    /**
     * This determines which system the BMS is attached to
     */
    SystemDetect& systemDetect;

    /**
     * Handler for reset CAN messages
     */
    ResetHandler& resetHandler;

    /**
     * This GPIO is used to represent when the system is ok. When this pin
     * is high, it represents that the BMS is in a state ready to
     * charge or discharge,
     */
    IO::GPIO& bmsOK;

    /**
     * Multiplexer to handle pack thermistors
     */
    DEV::ThermistorMux thermistorMux;

    /**
     * Internal watchdog to detect STM hang
     */
    EVT::core::DEV::IWDG& iwdg;

    /**
     * Boolean flag which represents that a state has just changed
     *
     * Useful for determining when operations that only take place once per
     * state change should take place.
     */
    bool stateChanged = false;

    /**
     * Utility variable that tracks the number of attempts made to read from the
     * BQ chip
     */
    uint8_t numBqAttemptsMade = 0;

    /**
     * Utility variable that tracks the number of attempts made to read safe
     * thermistor temperatures
     */
    uint8_t numThermAttemptsMade = 0;

    /**
     * Keeps track of the last time an attempt was made to communicate with the
     * BQ chip
     *
     * This is used in combination with BMS::numBqAttemptsMade to attempt BQ
     * communication a certain number of times with delay in attempts
     */
    uint32_t lastBqAttemptTime = 0;

    /**
     * Keeps track of the last time an attempt was made to read safe thermistor
     * temperatures
     *
     * This is used in combination with BMS::numThermAttemptsMade to read
     * thermistor temperatures a certain number of times with delay in attempts
     */
    uint32_t lastThermAttemptTime = 0;

    /**
     * Represents the total voltage read by the BQ chip
     *
     * This value is updated by reading the voltage from the BQ chip and is then
     * exposed over CANopen.
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
     * Stores important information about pack thermistor temperatures
     */
    PackTempInfo packTempInfo{
        .minPackTemp = 0,
        .minPackTempId = 0,
        .maxPackTemp = 0,
        .maxPackTempId = 0,
    };

    /**
     * Stores temperature information measured by the BQ
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
    CellVoltageInfo voltageInfo{
        .minCellVoltage = 0,
        .minCellVoltageId = 0,
        .maxCellVoltage = 0,
        .maxCellVoltageId = 0,
    };

    /**
     * Array that stores status information pulled from the BQ
     */
    uint8_t bqStatusArr[7] = {};

    /**
     * Value representing what errors have occurred on the BMS
     */
    uint8_t errorRegister = 0;

    /**
     * Value that tracks the ID of the last thermistor that was read
     */
    uint8_t lastCheckedThermNum = -1;

    /**
     * Handle the start of the state machine logic
     *
     * This considers the health of the system, and the existence of BQ
     * settings.
     *
     * State: State::START
     */
    void startState();

    /**
     * Handle holding the BMS in the initialization error state
     *
     * State: State::INITIALIZATION_ERROR
     */
    void initializationErrorState();

    /**
     * Handle the factory init state
     * This will check for the presence of settings having arrived over CANopen.
     *
     * State: State::FACTORY_INIT
     */
    void factoryInitState();

    /**
     * Handle the state where settings are actively being sent from the BMS to
     * the BQ chip
     *
     * State: State::TRANSFER_SETTINGS
     */
    void transferSettingsState();

    /**
     * Handle when the system is ready for charging/discharging
     *
     * Will poll health data and sensor information while waiting for input.
     *
     * State: State::SYSTEM_READY
     */
    void systemReadyState();

    /**
     * Handle when an unsafe condition is detected during normal operations
     *
     * State: State::UNSAFE_CONDITIONS_ERROR
     */
    void unsafeConditionsError();

    /**
     * Handle when the BMS is actively delivering power to the bike system
     *
     * State: State::POWER_DELIVERY
     */
    void powerDeliveryState();

    /**
     * Handle when the BMS is handling taking in charge
     *
     * State: State::CHARGING
     */
    void chargingState();

    /**
     * Check to see if the system is healthy
     *
     * This involves checking the ALARM pin, other status registers on the BQ,
     * and keeping track of the rest of the system.
     *
     * @return True if the system is healthy, false otherwise
     */
    bool isHealthy();

    /**
     * Update the local voltage variables with the values from the BQ chip
     *
     * This will iterate over all the values of interest and update them. These
     * values will then be able to be read over CANopen.
     *
     * This should be called when in a state where the BQ is ready and able
     * to read voltage. In other states, a call to this function should not
     * be present.
     */
    void updateBQData();

    /**
     * Read one thermistor value and report an over-temperature error if
     * necessary
     */
    void updateThermistorReading();

    /**
     * Clear the local voltage values (set to 0)
     *
     * This is to represent that the voltage readings are either not accurate or
     * not current. For states where the voltage read from the BQ is not
     * expected to be accurate or stable, this should be called so that the data
     * sent out over CANopen reflects that.
     */
    void clearVoltageReadings();

    /**
     * The object dictionary of the BMS
     *
     * Includes settings that determine how the BMS functions on the CANopen
     * network as well as the data that is exposed on the network.
     */
    CO_OBJ_T objectDictionary[OBJECT_DICTIONARY_SIZE + 1] = {
        MANDATORY_IDENTIFICATION_ENTRIES_1000_1014,
        HEARTBEAT_PRODUCER_1017(2000),
        IDENTITY_OBJECT_1018,
        SDO_CONFIGURATION_1200,

            //TPDO SETTINGS

        // TPDO0 settings
        TRANSMIT_PDO_SETTINGS_OBJECT_18XX(0, TRANSMIT_PDO_TRIGGER_TIMER, 0, 1000),

        // TPDO1 settings
        TRANSMIT_PDO_SETTINGS_OBJECT_18XX(1, TRANSMIT_PDO_TRIGGER_TIMER, 0, 1000),

        // TPDO2 settings
        TRANSMIT_PDO_SETTINGS_OBJECT_18XX(2, TRANSMIT_PDO_TRIGGER_TIMER, 0, 1000),

        // TPDO3 settings
        TRANSMIT_PDO_SETTINGS_OBJECT_18XX(3, TRANSMIT_PDO_TRIGGER_TIMER, 0, 1000),

        // TPDO4 settings
        TRANSMIT_PDO_SETTINGS_OBJECT_18XX(4, TRANSMIT_PDO_TRIGGER_TIMER, 0, 1000),

        // TPDO5 settings
        TRANSMIT_PDO_SETTINGS_OBJECT_18XX(5, TRANSMIT_PDO_TRIGGER_TIMER, 0, 1000),

        // TPDO6 settings
        TRANSMIT_PDO_SETTINGS_OBJECT_18XX(6, TRANSMIT_PDO_TRIGGER_TIMER, 0, 1000),

            //TPDO MAPPINGS

        // TPDO0 mapping, determines the PDO messages to send when TPDO1 is triggered
        TRANSMIT_PDO_MAPPING_START_KEY_1AXX(00,5),
            //batteryVoltage
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(00,1,PDO_MAPPING_UNSIGNED16),
            //minCellVoltage
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(00,2,PDO_MAPPING_UNSIGNED16),
            //minCellVoltageID
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(00,3,PDO_MAPPING_UNSIGNED8),
            //maxCellVoltage
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(00,4,PDO_MAPPING_UNSIGNED16),
            //maxCellVoltageID
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(00,5,PDO_MAPPING_UNSIGNED8),

        // TPDO1 mapping, determines the PDO messages to send when TPDO1 is triggered
        TRANSMIT_PDO_MAPPING_START_KEY_1AXX(01,7),
            //current
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(01,1,PDO_MAPPING_UNSIGNED16),
            //batteryPackMinTemp
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(01,2,PDO_MAPPING_UNSIGNED8),
            //batteryPackMinTempID
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(01,3,PDO_MAPPING_UNSIGNED8),
            //batteryPackMaxTemp
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(01,4,PDO_MAPPING_UNSIGNED8),
            //batteryPackMaxTempID
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(01,5,PDO_MAPPING_UNSIGNED8),
            //bqInternalTemp
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(01,6,PDO_MAPPING_UNSIGNED8),
            //state
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(01,7,PDO_MAPPING_UNSIGNED8),

        // TPDO2 mapping, determines the PDO messages to send when TPDO2 is triggered
        TRANSMIT_PDO_MAPPING_START_KEY_1AXX(02,8),
            //packtemp[0]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(02,1,PDO_MAPPING_UNSIGNED8),
            //packtemp[1]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(02,2,PDO_MAPPING_UNSIGNED8),
            //packtemp[2]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(02,3,PDO_MAPPING_UNSIGNED8),
            //packtemp[3]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(02,4,PDO_MAPPING_UNSIGNED8),
            //packtemp[4]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(02,5,PDO_MAPPING_UNSIGNED8),
            //packtemp[5]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(02,6,PDO_MAPPING_UNSIGNED8),
            //boardTemp1
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(02,7,PDO_MAPPING_UNSIGNED8),
            //boardTemp2
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(02,8,PDO_MAPPING_UNSIGNED8),


        // TPDO3 mapping, determines the PDO messages to send when TPDO3 is triggered
        TRANSMIT_PDO_MAPPING_START_KEY_1AXX(03,8),
            //errorRegister
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(03,1,PDO_MAPPING_UNSIGNED8),
            //bqStatusArr[0]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(03,2,PDO_MAPPING_UNSIGNED8),
            //bqStatusArr[1]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(03,3,PDO_MAPPING_UNSIGNED8),
            //bqStatusArr[2]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(03,4,PDO_MAPPING_UNSIGNED8),
            //bqStatusArr[3]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(03,5,PDO_MAPPING_UNSIGNED8),
            //bqStatusArr[4]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(03,6,PDO_MAPPING_UNSIGNED8),
            //bqStatusArr[5]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(03,7,PDO_MAPPING_UNSIGNED8),
            //bqStatusArr[6]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(03,8,PDO_MAPPING_UNSIGNED8),


        // TPDO4 mapping, determines the PDO messages to send when TPDO4 is triggered
        TRANSMIT_PDO_MAPPING_START_KEY_1AXX(04,4),
            //cellVoltage[0]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(04,1,PDO_MAPPING_UNSIGNED16),
            //cellVoltage[1]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(04,2,PDO_MAPPING_UNSIGNED16),
            //cellVoltage[2]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(04,3,PDO_MAPPING_UNSIGNED16),
            //cellVoltage[3]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(04,4,PDO_MAPPING_UNSIGNED16),


        // TPDO5 mapping, determines the PDO messages to send when TPDO5 is triggered
        TRANSMIT_PDO_MAPPING_START_KEY_1AXX(05,4),
            //cellVoltage[4]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(05,1,PDO_MAPPING_UNSIGNED16),
            //cellVoltage[5]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(05,2,PDO_MAPPING_UNSIGNED16),
            //cellVoltage[6]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(05,3,PDO_MAPPING_UNSIGNED16),
            //cellVoltage[7]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(05,4,PDO_MAPPING_UNSIGNED16),

        // TPDO6 mapping, determines the PDO messages to send when TPDO6 is triggered
        TRANSMIT_PDO_MAPPING_START_KEY_1AXX(06,4),
            //cellVoltage[8]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(06,1,PDO_MAPPING_UNSIGNED16),
            //cellVoltage[9]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(06,2,PDO_MAPPING_UNSIGNED16),
            //cellVoltage[10]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(06,3,PDO_MAPPING_UNSIGNED16),
            //cellVoltage[11]
            TRANSMIT_PDO_MAPPING_ENTRY_1AXX(06,4,PDO_MAPPING_UNSIGNED16),

            //TPDO LINKINGS

        // TPDO0 linking, links the TPDO entries to the variables they send
        DATA_LINK_START_KEY_21XX(00, 5),
            //batteryVoltage
            DATA_LINK_21XX(00,1, CO_TSIGNED16, &batteryVoltage),
            //minCellVoltage
            DATA_LINK_21XX(00,2,CO_TSIGNED16, &voltageInfo.minCellVoltage),
            //minCellVoltageId
            DATA_LINK_21XX(00,3,CO_TUNSIGNED8, &voltageInfo.minCellVoltageId),
            //maxCellVoltage
            DATA_LINK_21XX(00,4,CO_TSIGNED16, &voltageInfo.maxCellVoltage),
            //maxCellVoltageId
            DATA_LINK_21XX(00,5,CO_TUNSIGNED8, &voltageInfo.maxCellVoltageId),

        // TPDO1 linking, links the TPDO entries to the variables they send
        DATA_LINK_START_KEY_21XX(01,7),
            //current
            DATA_LINK_21XX(01,1,CO_TSIGNED16, &current),
            //batteryPackMinTemp
            DATA_LINK_21XX(01,2,CO_TUNSIGNED8, &packTempInfo.minPackTemp),
            //batteryPackMinTempId
            DATA_LINK_21XX(01,3,CO_TUNSIGNED8, &packTempInfo.minPackTempId),
            //batteryPackMaxTemp
            DATA_LINK_21XX(01,4,CO_TUNSIGNED8, &packTempInfo.maxPackTemp),
            //batteryPackMaxTempId
            DATA_LINK_21XX(01,5,CO_TUNSIGNED8, &packTempInfo.maxPackTempId),
            //bqInternalTemp
            DATA_LINK_21XX(01,6,CO_TUNSIGNED8, &bqTempInfo.internalTemp),
            //state
            DATA_LINK_21XX(01,7,CO_TUNSIGNED8, &state),

        // TPDO2 linking, links the TPDO entries to the variables they send
        DATA_LINK_START_KEY_21XX(02,8),
            //packtemp[0]
            DATA_LINK_21XX(02,1,CO_TUNSIGNED8, &thermistorTemperature[0]),
            //packtemp[1]
            DATA_LINK_21XX(02,2,CO_TUNSIGNED8, &thermistorTemperature[1]),
            //packtemp[2]
            DATA_LINK_21XX(02,3,CO_TUNSIGNED8, &thermistorTemperature[2]),
            //packtemp[3]
            DATA_LINK_21XX(02,4,CO_TUNSIGNED8, &thermistorTemperature[3]),
            //packtemp[4]
            DATA_LINK_21XX(02,5,CO_TUNSIGNED8, &thermistorTemperature[4]),
            //packtemp[5]
            DATA_LINK_21XX(02,6,CO_TUNSIGNED8, &thermistorTemperature[5]),
            //boardTemp1
            DATA_LINK_21XX(02,7,CO_TUNSIGNED8, &bqTempInfo.temp1),
            //boardTemp2
            DATA_LINK_21XX(02,8,CO_TUNSIGNED8, &bqTempInfo.temp2),

        // TPDO3 linking, links the TPDO entries to the variables they send
        DATA_LINK_START_KEY_21XX(03,8),
            //errorRegister
            DATA_LINK_21XX(03,1,CO_TUNSIGNED8, &errorRegister),
            //bqStatusArr[0]
            DATA_LINK_21XX(03,2,CO_TUNSIGNED8, &bqStatusArr[0]),
            //bqStatusArr[1]
            DATA_LINK_21XX(03,3,CO_TUNSIGNED8, &bqStatusArr[1]),
            //bqStatusArr[2]
            DATA_LINK_21XX(03,4,CO_TUNSIGNED8, &bqStatusArr[2]),
            //bqStatusArr[3]
            DATA_LINK_21XX(03,5,CO_TUNSIGNED8, &bqStatusArr[3]),
            //bqStatusArr[4]
            DATA_LINK_21XX(03,6,CO_TUNSIGNED8, &bqStatusArr[4]),
            //bqStatusArr[5]
            DATA_LINK_21XX(03,7,CO_TUNSIGNED8, &bqStatusArr[5]),
            //bqStatusArr[6]
            DATA_LINK_21XX(03,8,CO_TUNSIGNED8, &bqStatusArr[6]),

        // TPDO4 linking, links the TPDO entries to the variables they send
        DATA_LINK_START_KEY_21XX(04,4),
            //cellVoltage[0]
            DATA_LINK_21XX(04,1,CO_TUNSIGNED16, &cellVoltage[0]),
            //cellVoltage[1]
            DATA_LINK_21XX(04,2,CO_TUNSIGNED16, &cellVoltage[1]),
            //cellVoltage[2]
            DATA_LINK_21XX(04,3,CO_TUNSIGNED16, &cellVoltage[2]),
            //cellVoltage[3]
            DATA_LINK_21XX(04,4,CO_TUNSIGNED16, &cellVoltage[3]),

        // TPDO5 linking, links the TPDO entries to the variables they send
        DATA_LINK_START_KEY_21XX(05,4),
            //cellVoltage[4]
            DATA_LINK_21XX(05,1,CO_TUNSIGNED16, &cellVoltage[4]),
            //cellVoltage[5]
            DATA_LINK_21XX(05,2,CO_TUNSIGNED16, &cellVoltage[5]),
            //cellVoltage[6]
            DATA_LINK_21XX(05,3,CO_TUNSIGNED16, &cellVoltage[6]),
            //cellVoltage[7]
            DATA_LINK_21XX(05,4,CO_TUNSIGNED16, &cellVoltage[7]),

        // TPDO6 linking, links the TPDO entries to the variables they send
        DATA_LINK_START_KEY_21XX(06,4),
            //cellVoltage[8]
            DATA_LINK_21XX(06,1,CO_TUNSIGNED16, &cellVoltage[8]),
            //cellVoltage[9]
            DATA_LINK_21XX(06,2,CO_TUNSIGNED16, &cellVoltage[9]),
            //cellVoltage[10]
            DATA_LINK_21XX(06,3,CO_TUNSIGNED16, &cellVoltage[10]),
            //cellVoltage[11]
            DATA_LINK_21XX(06,4,CO_TUNSIGNED16, &cellVoltage[11]),
//TODO: BQ is implemented incorrectly, so we must fix it.
/*
        /// Expose information on the balancing of the target cells. Per
        /// cell ability to poll if the cell is balancing and write out
        /// balancing control.
        {
            .Key = CO_KEY(0x2107, 1, CO_OBJ____PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 1,
        },
        {
            .Key = CO_KEY(0x2107, 2,  CO_OBJ____PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 2,
        },
        {
            .Key = CO_KEY(0x2107, 3,  CO_OBJ____PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 3,
        },
        {
            .Key = CO_KEY(0x2107, 4,  CO_OBJ____PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 4,
        },

        {
            .Key = CO_KEY(0x2107, 5,  CO_OBJ____PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 5,
        },
        {
            .Key = CO_KEY(0x2107, 6,  CO_OBJ____PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 6,
        },
        {
            .Key = CO_KEY(0x2107, 7,  CO_OBJ____PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 7,
        },
        {
            .Key = CO_KEY(0x2107, 8,  CO_OBJ____PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 8,
        },
        {
            .Key = CO_KEY(0x2107, 9,  CO_OBJ____PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 9,
        },
        {
            .Key = CO_KEY(0x2107, 10,  CO_OBJ____PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 10,
        },
        {
            .Key = CO_KEY(0x2107, 11,  CO_OBJ____PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 11,
        },
        {
            .Key = CO_KEY(0x2107, 12,  CO_OBJ____PRW),
            .Type = ((CO_OBJ_TYPE*) &bq.balancingCANOpen),
            .Data = (uintptr_t) 12,
        },
*/
        // End of dictionary marker
        CO_OBJ_DICT_ENDMARK,
    };
};

}// namespace BMS
