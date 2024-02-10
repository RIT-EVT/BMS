/**
 * This test demonstrates the functionality of the ResetHandler class. To fully
 * test this, you will need to connect it to a CAN network and send it reset
 * messages.
 */

#include <ResetHandler.hpp>
#include <BMS.hpp>
#include <EVT/manager.hpp>
#include <EVT/utils/log.hpp>

namespace DEV = EVT::core::DEV;
namespace IO = EVT::core::IO;
namespace log = EVT::core::log;

/**
 * Interrupt handler for incoming CAN messages.
 *
 * @param priv[in] The private data (FixedQueue<CANOPEN_QUEUE_SIZE, CANMessage>)
 */
void canInterruptHandler(IO::CANMessage& message, void* priv) {
    auto* resetHandler = (BMS::ResetHandler*) priv;

    resetHandler->registerInput(message);

    log::LOGGER.log(log::Logger::LogLevel::INFO, "Message received");
}

int main() {
    EVT::core::platform::init();

    IO::UART& uart = IO::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(115200, true);

    uart.printf("\r\n\r\nReset Handler Test\r\n");

    log::LOGGER.setUART(&uart);
    log::LOGGER.setLogLevel(log::Logger::LogLevel::DEBUG);

    BMS::ResetHandler resetHandler;

    IO::CAN& can = IO::getCAN<BMS::BMS::CAN_TX_PIN, BMS::BMS::CAN_RX_PIN>();
    can.addIRQHandler(canInterruptHandler, &resetHandler);

    // Attempt to join the CAN network
    IO::CAN::CANStatus result = can.connect();

    if (result != IO::CAN::CANStatus::OK) {
        uart.printf("Failed to connect to the CAN network\r\n");
        return 1;
    }

    while (1) {
        if (resetHandler.shouldReset()) {
            log::LOGGER.log(log::Logger::LogLevel::INFO, "Reset triggered");
        }
        time::wait(1000);
    }
}
