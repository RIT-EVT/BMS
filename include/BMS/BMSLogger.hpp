#ifndef _BMS_LOGGER_
#define _BMS_LOGGER_

#include <EVT/io/UART.hpp>

namespace BMS {

/**
 * Logging utility that can print messages over UART. The printing can be
 * enabled or disabled depending on if the BMS is in debug mode
 */
class BMSLogger {
public:
    /**
     * The level at which messages should be logged at
     */
    enum class LogLevel {
        DEBUG = 0u,
        INFO = 1u,
        WARNING = 2u,
        ERROR = 3u
    };

    /**
     * Set the UART interface to use for printing messages
     *
     * @param uart[in] The reference to use for printing
     */
    void setUART(EVT::core::IO::UART* uart);

    /**
     * Set the level at which to log at.
     *
     * @param level[in] The level to log at
     */
    void setLogLevel(LogLevel level);

    /**
     * Log a message at the given level. If the UART interface has not been
     * provided, nothing will happen. Also if the level of the argument is
     * less then the current log level, nothing will happen.
     *
     * @param level[in] The log level of this particular message.
     * @param messageFormat[in] The print format to use
     */
    void log(LogLevel level, const char* format, ...);

private:
    /** UART to use for printing, nullptr means nothing will be logged */
    EVT::core::IO::UART* uart = nullptr;
    /**
     * The current log level limit, messages must be at or above this level to
     * be logged
     */
    LogLevel level = LogLevel::WARNING;
};

extern BMSLogger LOGGER;

}// namespace BMS

#endif
