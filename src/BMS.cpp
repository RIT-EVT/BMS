#include <BMS.hpp>

#include <EVT/utils/log.hpp>
#include <EVT/utils/time.hpp>
#include <cstring>

namespace time = EVT::core::time;
namespace log = EVT::core::log;

namespace BMS {

BMS::BMS(BQSettingsStorage& bqSettingsStorage, DEV::BQ76952 bq,
         DEV::Interlock& interlock, IO::GPIO& alarm,
         DEV::SystemDetect& systemDetect, IO::GPIO& bmsOK,
         DEV::ThermistorMux& thermMux, DEV::ResetHandler& resetHandler, EVT::core::DEV::IWDG& iwdg) : bqSettingsStorage(bqSettingsStorage),
                                        bq(bq), state(State::START), interlock(interlock),
                                        alarm(alarm), systemDetect(systemDetect), resetHandler(resetHandler),
                                        bmsOK(bmsOK), thermistorMux(thermMux), iwdg(iwdg), stateChanged(true) {
    bmsOK.writePin(IO::GPIO::State::LOW);

    updateBQData();
}

CO_OBJ_T* BMS::getObjectDictionary() {
    return objectDictionary;
}

uint16_t BMS::getObjectDictionarySize() {
    return OBJECT_DICTIONARY_SIZE;
}

void BMS::canTest() {
    batteryVoltage = 0x2301;
    voltageInfo = {
        0x6745,
        0x89,
        (int16_t) 0xcdab,
        0xef,
    };

    current = 0x2301;
    packTempInfo = {
        .minPackTemp = 0x45,
        .minPackTempId = 0x67,
        .maxPackTemp = 0x89,
        .maxPackTempId = 0xab,
    };
    bqTempInfo.internalTemp = 0xcd;
    state = static_cast<State>(0xef);

    thermistorTemperature[0] = 0x01;
    thermistorTemperature[1] = 0x23;
    thermistorTemperature[2] = 0x45;
    thermistorTemperature[3] = 0x67;
    thermistorTemperature[4] = 0x89;
    thermistorTemperature[5] = 0xab;
    bqTempInfo.temp1 = 0xcd;
    bqTempInfo.temp2 = 0xef;

    errorRegister = 0x01;
    bqStatusArr[0] = 0x23;
    bqStatusArr[1] = 0x45;
    bqStatusArr[2] = 0x67;
    bqStatusArr[3] = 0x89;
    bqStatusArr[4] = 0xab;
    bqStatusArr[5] = 0xcd;
    bqStatusArr[6] = 0xef;

    for (uint8_t i = 0; i < 12; i++) {
        switch (i % 4) {
        case 0:
            cellVoltage[i] = 0x2301;
            break;
        case 1:
            cellVoltage[i] = 0x6745;
            break;
        case 2:
            cellVoltage[i] = 0xab89;
            break;
        case 3:
            cellVoltage[i] = 0xefcd;
            break;
        }
    }
}

void BMS::process() {
    iwdg.refresh();

    switch (state) {
    case State::START:
        startState();
        break;
    case State::INITIALIZATION_ERROR:
        initializationErrorState();
        break;
    case State::FACTORY_INIT:
        factoryInitState();
        break;
    case State::TRANSFER_SETTINGS:
        transferSettingsState();
        break;
    case State::SYSTEM_READY:
        systemReadyState();
        break;
    case State::DEEP_SLEEP:
        break;
    case State::UNSAFE_CONDITIONS_ERROR:
        unsafeConditionsError();
        break;
    case State::POWER_DELIVERY:
        powerDeliveryState();
        break;
    case State::CHARGING:
        chargingState();
        break;
    }
}

void BMS::startState() {
    if (stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;

        // Reset all data
        numAttemptsMade = 0;
        numThermAttemptsMade = 0;
        lastAttemptTime = 0;
        lastThermAttemptTime = 0;
        clearVoltageReadings();
        current = 0;
        packTempInfo = {
            .minPackTemp = 0,
            .minPackTempId = 0,
            .maxPackTemp = 0,
            .maxPackTempId = 0,
        };
        bqTempInfo = {
            .internalTemp = 0,
            .temp1 = 0,
            .temp2 = 0,
        };
        memset(thermistorTemperature, 0, DEV::BQ76952::NUM_CELLS * sizeof(uint16_t));
        memset(bqStatusArr, 0, sizeof(uint8_t) * 3);
        errorRegister = 0;
        lastCheckedThermNum = -1;

        log::LOGGER.log(log::Logger::LogLevel::INFO, "Entering start state");
    }

    // Check if an error has taken place, and if so, check to make sure
    // a certain delay time has taken place before making another attempt
    if (numAttemptsMade > 0) {
        // If there has not been enough time between attempts, skip this run
        // of the state and try again later
        if ((time::millis() - lastAttemptTime) < ERROR_TIME_DELAY) {
            return;
        }
    }

    // Check to see if communication is possible with the BQ chip
    DEV::BQ76952::Status status = bq.communicationStatus();
    if (status != DEV::BQ76952::Status::OK) {

        // Increment the number of errors that have taken place
        numAttemptsMade++;

        // Record current time
        lastAttemptTime = time::millis();

        if (numAttemptsMade >= MAX_BQ_COMM_ATTEMPTS) {
            // If communication could not be handled, transition to error state
            state = State::INITIALIZATION_ERROR;
            errorRegister |= BQ_COMM_ERROR | static_cast<uint8_t>(status);
            stateChanged = true;
        }
    }
    // Check to see if we have setting to be transferred
    else if (bqSettingsStorage.hasSettings()) {
        state = State::TRANSFER_SETTINGS;
        stateChanged = true;
    }
    // Otherwise, no current settings, wait until setting are received
    else {
        state = State::FACTORY_INIT;
        stateChanged = true;
    }
}

void BMS::initializationErrorState() {
    if (stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;
        clearVoltageReadings();
        log::LOGGER.log(log::Logger::LogLevel::INFO, "Entering initialization error state");
    }

    updateThermistorReading();

    if (resetHandler.shouldReset()) {
        state = State::START;
        stateChanged = true;
    }
}

void BMS::factoryInitState() {
    if (stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;
        clearVoltageReadings();
        log::LOGGER.log(log::Logger::LogLevel::INFO, "Entering factory init state");
    }

    // Check to see if settings have come in, if so, go back to start state
    if (bqSettingsStorage.hasSettings()) {
        state = State::START;
        stateChanged = true;
    }
}

void BMS::transferSettingsState() {
    if (stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        bqSettingsStorage.resetTransfer();
        numAttemptsMade = 0;
        stateChanged = false;
        clearVoltageReadings();
        log::LOGGER.log(log::Logger::LogLevel::INFO, "Entering transfer settings state");
    }

    // Check if an error has taken place, and if so, check to make sure
    // a certain delay time has taken place before making another attempt
    if (numAttemptsMade > 0) {
        // If there has not been enough time between attempts, skip this run
        // of the state and try again later
        if ((time::millis() - lastAttemptTime) < ERROR_TIME_DELAY) {
            return;
        }
    }

    bool isComplete = false;
    auto result = bqSettingsStorage.transferSetting(isComplete);
    if (result != DEV::BQ76952::Status::OK) {
        numAttemptsMade++;

        // If the number of errors are over the max
        if (numAttemptsMade >= MAX_BQ_COMM_ATTEMPTS) {
            // If the settings did not transfer successfully, transition to
            // error state
            state = State::INITIALIZATION_ERROR;
            errorRegister |= BQ_COMM_ERROR | static_cast<uint8_t>(result);
            stateChanged = true;
        }

        lastAttemptTime = time::millis();

        bqSettingsStorage.resetTransfer();

    } else if (isComplete) {
        iwdg.init();
        state = State::SYSTEM_READY;
        stateChanged = true;
    }
}

void BMS::systemReadyState() {
    if (stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;
        log::LOGGER.log(log::Logger::LogLevel::INFO, "Entering system ready state");
    }

    // TODO: Check for need to deep sleep and enter deep sleep mode

    // TODO: Update error register of BMS
    if (!isHealthy()) {
        state = State::UNSAFE_CONDITIONS_ERROR;
        stateChanged = true;
        return;
    }

    if (interlock.isDetected()) {
        if (systemDetect.getIdentifiedSystem() == DEV::SystemDetect::System::BIKE) {
            state = State::POWER_DELIVERY;
            stateChanged = true;
            return;
        } else if (systemDetect.getIdentifiedSystem() == DEV::SystemDetect::System::CHARGER) {
            state = State::CHARGING;
            stateChanged = true;
            return;
        }
    }

    updateBQData();
    updateThermistorReading();
}

void BMS::unsafeConditionsError() {
    if (stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;
        log::LOGGER.log(log::Logger::LogLevel::INFO, "Entering unsafe conditions state");
    }

    updateBQData();
    updateThermistorReading();

    if (resetHandler.shouldReset()) {
        state = State::START;
        stateChanged = true;
    }
}

void BMS::powerDeliveryState() {
    if (stateChanged) {
        bmsOK.writePin(BMS_OK);
        stateChanged = false;
        log::LOGGER.log(log::Logger::LogLevel::INFO, "Entering power delivery state");
    }

    // TODO: Update error register of BMS
    if (!isHealthy()) {
        state = State::UNSAFE_CONDITIONS_ERROR;
        stateChanged = true;
        return;
    }

    if (!interlock.isDetected()) {
        state = State::SYSTEM_READY;
        stateChanged = true;
        return;
    }

    updateBQData();
    updateThermistorReading();
}

void BMS::chargingState() {
    if (stateChanged) {
        bmsOK.writePin(BMS_OK);
        stateChanged = false;
        log::LOGGER.log(log::Logger::LogLevel::INFO, "Entering charging state");
    }

    // TODO: Update error register of BMS
    if (!isHealthy()) {
        state = State::UNSAFE_CONDITIONS_ERROR;
        stateChanged = true;
        return;
    }

    if (!interlock.isDetected()) {
        state = State::SYSTEM_READY;
        stateChanged = true;
        return;
    }

    updateBQData();
    updateThermistorReading();
}

bool BMS::isHealthy() {
    if (alarm.readPin() == ALARM_ACTIVE_STATE) {
        errorRegister |= BQ_ALARM_ERROR;
    } else if ((errorRegister & 0xF0) > 0) {
        errorRegister |= BQ_COMM_ERROR;
    }

    return errorRegister == 0;
}

void BMS::updateBQData() {
    // TODO: Limit the number of times this is called, currently this
    //       `updateVoltageReadings` is called every run of the loop
    //       which results in a lot of I2C calls. This isn't directly an
    //       issue, just not necessary. Could be limited to update once a
    //       second.


    // Check if an error has taken place, and if so, check to make sure
    // a certain delay time has taken place before making another attempt
    if (numAttemptsMade > 0) {
        // If there has not been enough time between attempts, skip this run
        // of the state and try again later
        if ((time::millis() - lastAttemptTime) < ERROR_TIME_DELAY) {
            return;
        }
    }

    DEV::BQ76952::Status result = bq.getCellVoltage(cellVoltage, totalVoltage, voltageInfo);

    if (result == DEV::BQ76952::Status::OK) {
        result = bq.getVoltage(batteryVoltage);
    }

    if (result == DEV::BQ76952::Status::OK) {
        result = bq.getCurrent(current);
    }

    if (result == DEV::BQ76952::Status::OK) {
        result = bq.getTemps(bqTempInfo);
    }

    if (result == DEV::BQ76952::Status::OK) {
        result = bq.getBQStatus(bqStatusArr);
    }

    if (result != DEV::BQ76952::Status::OK) {
        numAttemptsMade++;

        // If the number of errors are over the max
        if (numAttemptsMade >= MAX_BQ_COMM_ATTEMPTS) {
            errorRegister |= static_cast<uint8_t>(result);
            return;
        }

        lastAttemptTime = time::millis();
    } else {
        numAttemptsMade = 0;
    }
}

void BMS::updateThermistorReading() {
    // Check if an error has taken place, and if so, check to make sure
    // a certain delay time has taken place before making another attempt
    if (numThermAttemptsMade > 0) {
        // If there has not been enough time between attempts, skip this run
        // of the state and try again later
        if ((time::millis() - lastThermAttemptTime) < ERROR_TIME_DELAY) {
            return;
        }
    }

    lastCheckedThermNum = (lastCheckedThermNum + 1) % NUM_THERMISTORS;
    thermistorTemperature[lastCheckedThermNum] = thermistorMux.getTemp(lastCheckedThermNum);

    packTempInfo.maxPackTempId = 0;
    packTempInfo.minPackTempId = 0;
    packTempInfo.maxPackTemp = thermistorTemperature[0];
    packTempInfo.minPackTemp = thermistorTemperature[0];
    for (uint8_t i = 1; i < NUM_THERMISTORS; i++) {
        if (thermistorTemperature[i] < packTempInfo.minPackTemp) {
            packTempInfo.minPackTemp = thermistorTemperature[i];
            packTempInfo.minPackTempId = i;
        } else if (thermistorTemperature[i] > packTempInfo.maxPackTemp) {
            packTempInfo.maxPackTemp = thermistorTemperature[i];
            packTempInfo.maxPackTempId = i;
        }
    }

    if (thermistorTemperature[lastCheckedThermNum] > MAX_THERM_TEMP) {
        numThermAttemptsMade++;

        if(numThermAttemptsMade >= MAX_THERM_READ_ATTEMPTS) {
            log::LOGGER.log(log::Logger::LogLevel::ERROR, "Thermistor %d over max temp: %d", lastCheckedThermNum, thermistorTemperature[lastCheckedThermNum]);

            errorRegister |= OVER_TEMP_ERROR;
            return;
        }

        lastThermAttemptTime = time::millis();
        lastCheckedThermNum--;
    } else {
        numThermAttemptsMade = 0;
    }
}

void BMS::clearVoltageReadings() {
    totalVoltage = 0;
    batteryVoltage = 0;
    voltageInfo = { 0, 0, 0, 0 };

    // Zero out all cell voltages
    memset(cellVoltage, 0, DEV::BQ76952::NUM_CELLS * sizeof(uint16_t));
}

}// namespace BMS
