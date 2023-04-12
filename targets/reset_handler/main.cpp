/**
*
*/

#include <BMS.hpp>
#include <dev/ResetHandler.hpp>
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
    auto* resetHandler = (BMS::DEV::ResetHandler*) priv;

    resetHandler->registerInput(message);

    log::LOGGER.log(log::Logger::LogLevel::INFO, "Message received");
}


int main() {
    EVT::core::platform::init();

    IO::UART& uart = IO::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(115200, true);

    uart.printf("\r\n\r\nReset Handler Test\r\n");

    log::LOGGER.setUART(&uart);
    log::LOGGER.setLogLevel(log::Logger::LogLevel::DEBUG);

    BMS::DEV::ResetHandler resetHandler;

    IO::CAN& can = IO::getCAN<BMS::BMS::CAN_TX_PIN, BMS::BMS::CAN_RX_PIN>();
    can.addIRQHandler(canInterruptHandler, &resetHandler);

    while (1) {
        if (resetHandler.shouldReset()) {
            log::LOGGER.log(log::Logger::LogLevel::INFO, "Reset triggered");
        }
        time::wait(1000);
    }
}