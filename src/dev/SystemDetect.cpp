#include <BMS/dev/SystemDetect.hpp>

#include <EVT/utils/time.hpp>

namespace BMS::DEV {

SystemDetect::SystemDetect(uint32_t bikeHeartBeat, uint32_t chargeHeatBeat,
                           uint32_t timout) {
    this->bikeHeatBeat = bikeHeatBeat;
    this->chargeHeatBeat = chargeHeatBeat;
    this->timeout = timeout;
    this->lastRead = 0;
    this->identifiedSystem = System::UNKNOWN;
}

void SystemDetect::processHeatBeat(int heartBeatID) {
    switch (heartBeatID) {
    case bikeHeatBeat:
        identifiedSystem = System::BIKE;
        lastRead = EVT::core::time::millis();
        break;
    case chargeHeatBeat:
        identifiedSystem = System::CHARGER;
        lastRead = EVT::core::time::millis();
        break;
    }
}

SystemDetect::System SystemDetect::getIdentifiedSystem() {
    // Check for timeout
    if ((EVT::core::time::millis() - lastRead) > timeout) {
        return System::UNKNOWN;
    }
    return identifiedSystem;
}

}// namespace BMS::DEV
