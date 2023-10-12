#include <cstring>
#include <dev/ResetHandler.hpp>

namespace BMS::DEV {

ResetHandler::ResetHandler() = default;

void ResetHandler::registerInput(IO::CANMessage msg) {
    lastLoadedMsg = (lastLoadedMsg + 1) % MSG_HIST_LEN;
    msgHistory[lastLoadedMsg] = msg;
}

bool ResetHandler::shouldReset() {
    uint8_t resetArr[RESET_ARR_LEN] = RESET_ARR;
    for (IO::CANMessage msg : msgHistory) {
        if (msg.getId() != RESET_ID || msg.getDataLength() != RESET_ARR_LEN) {
            return false;
        }
        uint8_t* payload = msg.getPayload();
        for (uint8_t i = 0; i < MSG_HIST_LEN; i++) {
            if (payload[i] != resetArr[i]) {
                return false;
            }
        }
    }

    // Clear message history
    for (uint8_t i = 0; i < MSG_HIST_LEN; i++) {
        msgHistory[i] = IO::CANMessage();
    }

    return true;
}

}// namespace BMS::DEV
