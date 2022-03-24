#include <BMS/BMS.hpp>
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
    stateChanged = false;
}

CO_OBJ_T* BMS::getObjectDictionary() {
    return &objectDictionary[0];
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
    if(stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;
    }

    // Check to see if communication is possible with the BQ chip
    // TODO: Try this n number of times before failing
    if (bq.communicationStatus() != DEV::BQ76952::Status::OK) {
        // If communication could not be handled, transition to error state
        // TODO: Update error mapping with error information
        state = State::INITIALIZATION_ERROR;
        stateChanged = true;
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
    if(stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;
    }
}

void BMS::factoryInitState() {
    if(stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;
    }

    // Check to see if settings have come in, if so, go back to start state
    if (bqSettingsStorage.hasSettings()) {
        state = State::START;
        stateChanged = true;
    }
}

void BMS::transferSettingsState() {
    if(stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;
    }

    // TODO: Attempt n number of times before failing
    auto result = bqSettingsStorage.transferSettings();
    if (result != DEV::BQ76952::Status::OK) {
        // If the settings did not transfer successfully, transiton tp
        // error state
        // TODO: Update error mapping with error information
        state = State::INITIALIZATION_ERROR;
        stateChanged = true;
    } else {
        // Otherwise, move on to ready state
        state = State::SYSTEM_READY;
        stateChanged = true;
    }
}

void BMS::systemReadyState() {
    if(stateChanged) {
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
        } else if (systemDetect.getIdentifiedSystem() == DEV::SystemDetect::System::CHARGER) {
            state = State::CHARGING;
            stateChanged = true;
        }
    }
}

void BMS::unsafeConditionsError() {
    if(stateChanged) {
        bmsOK.writePin(BMS_NOT_OK);
        stateChanged = false;
    }
}

void BMS::powerDeliveryState() {
    if(stateChanged) {
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
    }
}

void BMS::chargingState() {
    if(stateChanged) {
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
    }
}

bool BMS::isHealthy() {
    return alarm.readPin() != ALARM_ACTIVE_STATE;
}

}// namespace BMS
