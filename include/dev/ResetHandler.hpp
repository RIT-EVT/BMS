#pragma once

#include <EVT/io/types/CANMessage.hpp>

#define MSG_HIST_LEN 5
#define RESET_ID 0x7FF
#define RESET_ARR_LEN 8
#define RESET_ARR \
    { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }

namespace IO = EVT::core::IO;

namespace BMS::DEV {

/**
 *
 */
class ResetHandler {
public:
    /**
     *
     */
    ResetHandler();

    /**
     *
     * @param msg
     */
    void registerInput(IO::CANMessage msg);

    /**
     *
     * @return
     */
    bool shouldReset();

private:
    IO::CANMessage msgHistory[MSG_HIST_LEN] = {};
    uint8_t lastLoadedMsg = 0;
};

}// namespace BMS::DEV
