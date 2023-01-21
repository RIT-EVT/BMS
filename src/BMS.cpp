#include <BMS/BMS.hpp>

#include <EVT/utils/time.hpp>

#include <stdint.h>

namespace BMS {

BMS::BMS(BQSettingsStorage& bqSettingsStorage, DEV::BQ76952 bq,
         DEV::Interlock& interlock, EVT::core::IO::GPIO& alarm,
         DEV::SystemDetect& systemDetect, EVT::core::IO::GPIO& bmsOK) : bqSettingsStorage(bqSettingsStorage),
                                                                        bq(bq),
                                                                        interlock(interlock),
                                                                        alarm(alarm),
                                                                        systemDetect(systemDetect),
                                                                        bmsOK(bmsOK) {

    state = State::START;
    bmsOK.writePin(EVT::core::IO::GPIO::State::LOW);
    stateChanged = true;

    updateVoltageReadings();
}



CO_OBJ_T* BMS::getObjectDictionary() {
        return objectDictionaryOne;
}

uint16_t BMS::getObjectDictionarySize() {
    return OBJECT_DIRECTIONARY_SIZE;
}

void BMS::process() {
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
        numAttemptsMade = 0;
        stateChanged = false;
        clearVoltageReadings();
    }

    // Check if an error has taken place, and if so, check to make sure
    // a certain delay time has taken place before making another attempt
    if (numAttemptsMade > 0) {
        // If there has not been enough time between attempts, skip this run
        // of the state and try again later
        if ((EVT::core::time::millis() - lastAttemptTime) < ERROR_TIME_DELAY) {
            return;
        }
    }

    // Check to see if communication is possible with the BQ chip
    if (bq.communicationStatus() != DEV::BQ76952::Status::OK) {

        // Increment the number of errors that have taken place
        numAttemptsMade++;

        // Record current time
        lastAttemptTime = EVT::core::time::millis();

        if (numAttemptsMade >= MAX_BQ_COMM_ATTEMPTS) {
            // If communication could not be handled, transition to error state
            // TODO: Update error mapping with error information
            state = State::INITIALIZATION_ERROR;
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
    }
}

void BMS::factoryInitState() {
    if (stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;
        clearVoltageReadings();
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
        bqSettingsStorage.resetTranfer();
        numAttemptsMade = 0;
        stateChanged = false;
        //VoltageReadings();
    }

    // Check if an error has taken place, and if so, check to make sure
    // a certain delay time has taken place before making another attempt
    if (numAttemptsMade > 0) {
        // If there has not been enough time between attempts, skip this run
        // of the state and try again later
        if ((EVT::core::time::millis() - lastAttemptTime) < ERROR_TIME_DELAY) {
            return;
        }
    }

    bool isComplete = false;
    auto result = bqSettingsStorage.transferSetting(isComplete);
    if (result != DEV::BQ76952::Status::OK) {
        numAttemptsMade++;

        // If the number of errors are over the max
        if (numAttemptsMade >= MAX_BQ_COMM_ATTEMPTS) {
            // If the settings did not transfer successfully, transiton tp
            // error state
            // TODO: Update error mapping with error information
            state = State::INITIALIZATION_ERROR;
            stateChanged = true;
        }

        lastAttemptTime = EVT::core::time::millis();

        bqSettingsStorage.resetTranfer();

    } else if (isComplete) {
        state = State::SYSTEM_READY;
        stateChanged = true;
    }
}

void BMS::systemReadyState() {
    if (stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;
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

    updateVoltageReadings();
}

void BMS::unsafeConditionsError() {
    if (stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;
    }

    updateVoltageReadings();
}

void BMS::powerDeliveryState() {
    if (stateChanged) {
        bmsOK.writePin(BMS_OK);
        stateChanged = false;
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

    updateVoltageReadings();
}

void BMS::chargingState() {
    if (stateChanged) {
        bmsOK.writePin(BMS_OK);
        stateChanged = false;
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

    updateVoltageReadings();
}

bool BMS::isHealthy() {
    return alarm.readPin() != ALARM_ACTIVE_STATE;
}

void BMS::updateVoltageReadings() {
    // TODO: Handle when an error has taken place
    for(int8_t i = 0; i < DEV::BQ76952::NUM_CELLS; i++){
        cellVoltage[i]= i;
    }
    bq.getCellVoltage(cellVoltage, totalVoltage, voltageInfo);
    EVT::core::time::wait(10);
}

void BMS::clearVoltageReadings() {
//    totalVoltage = 0;
//
//    // Zero out each cell voltage
//    for (uint8_t i = 0; i < DEV::BQ76952::NUM_CELLS; i++) {
//        cellVoltage[i] = 0;
//    }
}

}// namespace BMS
