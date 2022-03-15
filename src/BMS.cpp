#include <BMS/BMS.hpp>
#include <stdint.h>

namespace BMS {

BMS::BMS(BQSettingsStorage& bqSettingsStorage, DEV::BQ76952 bq) : bqSettingsStorage(bqSettingsStorage), bq(bq) {
}

CO_OBJ_T* BMS::getObjectDictionary() {
    return &objectDictionary[0];
}

uint16_t BMS::getObjectDictionarySize() {
    return OBJECT_DIRECTIONARY_SIZE;
}

void BMS::process() {
    switch(state) {
        case State::START:
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
    if (bq.communicationStatus() != DEV::BQ76952::Status::OK) {
        // If communication could not be handled, transition to error state
        // TODO: Update error mapping with error information
        state = State::INITIALIZATION_ERROR;
    }
    else {
        // Otherwise, move on to transferring settings
        state = State::TRANSFER_SETTINGS;
    }
}

void BMS::initializationErrorState() {

}

void BMS::factoryInitState() {

}

void BMS::transferSettingsState() {

}

void BMS::systemReadyState() {

}

void BMS::unsafeConditionsError() {

}

void BMS::powerDeliveryState() {

}

void BMS::chargingState() {

}

}// namespace BMS
