#include <ResetHandler.hpp>

namespace io = core::io;

namespace BMS {

ResetHandler::ResetHandler() = default;

void ResetHandler::registerInput(io::CANMessage msg) {
    lastRegMsgIndex = (lastRegMsgIndex + 1) % MSG_HIST_LEN;
    msgHistory[lastRegMsgIndex] = msg;
}

bool ResetHandler::shouldReset() {
    for (io::CANMessage msg : msgHistory) {
        if (msg.getId() != RESET_ID || msg.getDataLength() != RESET_ARR_LEN) {
            return false;
        }

        uint8_t* payload = msg.getPayload();
        for (uint8_t i = 0; i < RESET_ARR_LEN; i++) {
            if (payload[i] != RESET_ARR[i]) {
                return false;
            }
        }
    }

    // Clear message history
    for (uint8_t i = 0; i < MSG_HIST_LEN; i++) {
        msgHistory[i] = io::CANMessage();
    }

    return true;
}

}// namespace BMS
