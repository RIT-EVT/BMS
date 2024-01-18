#include "SystemDetect.hpp"

#include "EVT/utils/time.hpp"

namespace time = EVT::core::time;

namespace BMS {

SystemDetect::SystemDetect(uint32_t bikeHeartbeat, uint32_t chargeHeartbeat,
                           uint32_t timeout) {
    this->bikeHeartBeat = bikeHeartbeat;
    this->chargeHeartbeat = chargeHeartbeat;
    this->timeout = timeout;
    this->lastRead = 0;
    this->identifiedSystem = System::UNKNOWN;
}

void SystemDetect::processHeartbeat(uint32_t heartbeatID) {
    if (heartbeatID == bikeHeartBeat) {
        identifiedSystem = System::BIKE;
        lastRead = time::millis();
    } else if (heartbeatID == chargeHeartbeat) {
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
