#pragma once

#include <EVT/io/types/CANMessage.hpp>

namespace BMS {

/**
 * Detects and reports reset CAN messages
 */
class ResetHandler {
public:
    /**
     * Make new handler instance
     */
    ResetHandler();

    /**
     * Register a received CAN message
     *
     * @param msg Message to register
     */
    void registerInput(EVT::core::IO::CANMessage msg);

    /**
     * Check whether reset messages have been received, indicating that the BMS
     * should reset
     *
     * @return Whether the BMS should reset
     */
    bool shouldReset();

private:
    /** Number of reset message frames required to trigger a reset */
    static constexpr uint8_t MSG_HIST_LEN = 5;
    /** Reset message ID */
    static constexpr uint16_t RESET_ID = 0x7FF;;
    /** Reset message array length */
    static constexpr uint8_t RESET_ARR_LEN = 8;;
    /** Reset message array */
    static constexpr uint8_t const RESET_ARR[8] =
        { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };

    /** Array of last MSG_HIST_LEN messages received */
    EVT::core::IO::CANMessage msgHistory[MSG_HIST_LEN] = {};
    /** Index of the last registered message */
    uint8_t lastRegMsgIndex = 0;
};

}// namespace BMS
