#include <EVT/io/manager.hpp>
#include <BMS/BMSLogger.hpp>

namespace IO = EVT::core::IO;

int main() {
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);
    BMS::LOGGER.setUART(&uart);

}
