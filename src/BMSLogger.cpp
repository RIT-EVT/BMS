#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <BMS/BMSLogger.hpp>

namespace BMS {
// Instance of the global logger that all objects can use.
BMSLogger LOGGER;

void BMSLogger::setUART(EVT::core::IO::UART* uart) {
    this->uart = uart;
}

void BMSLogger::setLogLevel(LogLevel level) {
    this->level = level;
}

void BMSLogger::log(LogLevel level, const char* format, ...) {
    // If there isn't a UART interface, cannot print
    if(!uart)
        return;
    // If the log level is too low, don't print
    if(level < this->level)
        return;

    switch(level) {
        case LogLevel::DEBUG:
            uart->printf("DEBUG::");
            break;
        case LogLevel::INFO:
            uart->printf("INFO::");
            break;
        case LogLevel::WARNING:
            uart->printf("WARNING::");
            break;
        case LogLevel::ERROR:
            uart->printf("ERROR::");
            break;
    }

    va_list args;
    va_start(args, format);

    char string[200];
    vsprintf(string, format, args);
    uart->printf("%s\r\n", string);

    va_end(args);
}

}  // namespace BMS
