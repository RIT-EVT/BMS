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
