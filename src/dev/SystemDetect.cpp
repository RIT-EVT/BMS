#include <dev/SystemDetect.hpp>

#include <EVT/utils/time.hpp>

namespace time = EVT::core::time;

namespace BMS::DEV {

SystemDetect::SystemDetect(uint32_t bikeHeartBeat, uint32_t chargeHeartBeat,
                           uint32_t timeout) {
    this->bikeHeartBeat = bikeHeartBeat;
    this->chargeHeartBeat = chargeHeartBeat;
    this->timeout = timeout;
    this->lastRead = 0;
    this->identifiedSystem = System::UNKNOWN;
}

void SystemDetect::processHeartBeat(uint32_t heartBeatID) {
    if (heartBeatID == bikeHeartBeat) {
        identifiedSystem = System::BIKE;
        lastRead = time::millis();
    } else if (heartBeatID == chargeHeartBeat) {
        identifiedSystem = System::CHARGER;
        lastRead = time::millis();
    }
}

SystemDetect::System SystemDetect::getIdentifiedSystem() {
    // Check for timeout
    if ((time::millis() - lastRead) > timeout) {
        return System::UNKNOWN;
    }
    return identifiedSystem;
}

}// namespace BMS::DEV
