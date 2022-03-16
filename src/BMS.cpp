#include <BMS/BMS.hpp>
#include <stdint.h>

namespace BMS {

BMS::BMS(BQSettingsStorage& bqSettingsStorage, DEV::BQ76952 bq, DEV::Interlock& interlock, EVT::core::IO::GPIO& alarm) :
    bqSettingsStorage(bqSettingsStorage),
    bq(bq),
    interlock(interlock),
    alarm(alarm) {
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
        break;
    case State::FACTORY_INIT:
        break;
    case State::TRANSFER_SETTINGS:
        break;
    case State::SYSTEM_READY:
        break;
    case State::DEEP_SLEEP:
        break;
    case State::UNSAFE_CONDITIONS_ERROR:
        break;
    case State::POWER_DELIVERY:
        break;
    case State::CHARGING:
        break;
    }
}

void BMS::startState() {
    // Check to see if communication is possible with the BQ chip
    // TODO: Try this n number of times before failing
    if (bq.communicationStatus() != DEV::BQ76952::Status::OK) {
        // If communication could not be handled, transition to error state
        // TODO: Update error mapping with error information
        state = State::INITIALIZATION_ERROR;
    } else {
        // Otherwise, move on to transferring settings
        state = State::TRANSFER_SETTINGS;
    }
}

void BMS::initializationErrorState() {
}

void BMS::factoryInitState() {
}

void BMS::transferSettingsState() {
    // TODO: Attempt n number of times before failing
    auto result = bqSettingsStorage.transferSettings();
    if (result != DEV::BQ76952::Status::OK) {
        // If the settings did not transfer successfully, transiton tp
        // error state
        // TODO: Update error mapping with error information
        state = State::INITIALIZATION_ERROR;
    } else {
        // Otherwise, move on to ready state
        state = State::SYSTEM_READY;
    }
}

void BMS::systemReadyState() {
    // TODO: Check for need to deep sleep and enter deep sleep mode

    // TODO: Update error register of BMS
    if(!isHealthy()) {
        state = State::UNSAFE_CONDITIONS_ERROR;
        return;
    }

    // TODO: Determine if the BMS is on the bike or the charger
    if(interlock.isDetected()) {
        // Transition to providing power
        state = State::POWER_DELIVERY;
    }
}

void BMS::unsafeConditionsError() {
}

void BMS::powerDeliveryState() {
    // TODO: Update error register of BMS
    if(!isHealthy()) {
        state = State::UNSAFE_CONDITIONS_ERROR;
        return;
    }

    if(!interlock.isDetected()) {
        state = State::SYSTEM_READY;
    }
}

void BMS::chargingState() {
    // TODO: Update error register of BMS
    if(!isHealthy()) {
        state = State::UNSAFE_CONDITIONS_ERROR;
        return;
    }

    if(!interlock.isDetected()) {
        state = State::SYSTEM_READY;
    }
}

bool BMS::isHealthy() {
    return alarm.readPin() != ALARM_ACTIVE_STATE;
}

}// namespace BMS
