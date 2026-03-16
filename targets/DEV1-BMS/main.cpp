/**
 * This is the main target to be used for the BMS in the DEV1 battery packs
 */

#include <EVT/io/CANOpenMacros.hpp>
#include <EVT/io/CANopen.hpp>
#include <EVT/io/UART.hpp>
#include <EVT/io/types/CANMessage.hpp>
#include <EVT/manager.hpp>

#include <core/dev/storage/EEPROM.hpp>
#include <core/dev/storage/M24C32.hpp>

#include <core/utils/log.hpp>
#include <core/utils/types/FixedQueue.hpp>

#include <BMS.hpp>
#include <SystemDetect.hpp>
#include <dev/BQ76952.hpp>

namespace IO = core::io;
namespace DEV = core::dev;
namespace time = core::time;
namespace log = core::log;

#define BIKE_HEART_BEAT 0x70A   // NODE_ID = 10
#define CHARGER_HEART_BEAT 0x710// NODE_ID = 16
#define DETECT_TIMEOUT 1000

/**
 * This struct is a catchall for data that is needed by the CAN interrupt
 * handler. An instance of this struct will be provided as the parameter
 * to the interrupt handler.
 */
struct CANInterruptParams {
    core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage>* queue;
    BMS::SystemDetect* systemDetect;
    BMS::ResetHandler* resetHandler;
};

/**
 * Interrupt handler for incoming CAN messages.
 *
 * @param priv[in] The private data (FixedQueue<CANOPEN_QUEUE_SIZE, CANMessage>)
 */
void canInterruptHandler(IO::CANMessage& message, void* priv) {
    struct CANInterruptParams* params = (CANInterruptParams*) priv;

    core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage>* queue = params->queue;
    BMS::SystemDetect* systemDetect = params->systemDetect;
    BMS::ResetHandler* resetHandler = params->resetHandler;

    systemDetect->processHeartbeat(message.getId());

    resetHandler->registerInput(message);

    if (queue == nullptr)
        return;
    if (!message.isCANExtended())
        queue->append(message);
}

int main() {
    // Initialize system
    core::platform::init();

    // Queue that will store CANopen messages
    core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage> canOpenQueue;

    // Initialize the system detect
    BMS::SystemDetect systemDetect(BIKE_HEART_BEAT, CHARGER_HEART_BEAT,
                                   DETECT_TIMEOUT);

    BMS::ResetHandler resetHandler;

    // Create struct that will hold CAN interrupt parameters
    CANInterruptParams canParams = {
        .queue = &canOpenQueue,
        .systemDetect = &systemDetect,
        .resetHandler = &resetHandler,
    };

    // Initialize IO
    // TODO: Investigate adding CAN filters
    IO::CAN& can = IO::getCAN<BMS::BMS::CAN_TX_PIN, BMS::BMS::CAN_RX_PIN>();
    can.addIRQHandler(canInterruptHandler, &canParams);

    IO::UART& uart = IO::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(115200);
    IO::I2C& i2c = IO::getI2C<BMS::BMS::I2C_SCL_PIN, BMS::BMS::I2C_SDA_PIN>();

    // Initialize the timer
    DEV::Timer& timer = DEV::getTimer<DEV::MCUTimer::Timer2>(100);

    // Initialize the EEPROM
    DEV::M24C32 eeprom(0x57, i2c);

    // Initialize the logger
    log::LOGGER.setUART(&uart);
    log::LOGGER.setLogLevel(log::Logger::LogLevel::INFO);

    // Initialize the BQ interfaces
    IO::GPIO& bqReset = IO::getGPIO<BMS::BMS::BQ_RESET_PIN>();
    BMS::dev::BQ76952 bq(i2c, 0x08, bqReset);
    BMS::BQSettingsStorage bqSettingsStorage(eeprom, bq);

    // Initialize the Interlock
    IO::GPIO& interlockGPIO = IO::getGPIO<BMS::BMS::INTERLOCK_PIN>(IO::GPIO::Direction::INPUT);
    BMS::dev::Interlock interlock(interlockGPIO);

    // Initialize the alarm pin
    IO::GPIO& alarm = IO::getGPIO<BMS::BMS::ALARM_PIN>(IO::GPIO::Direction::INPUT);

    // Initialize the system OK pin
    IO::GPIO& bmsOK = IO::getGPIO<BMS::BMS::OK_PIN>(IO::GPIO::Direction::OUTPUT);

    // Initialize the error LED pin
    IO::GPIO& errorLed = IO::getGPIO<BMS::BMS::ERROR_LED_PIN>(IO::GPIO::Direction::OUTPUT);

    // Initialize the thermistor MUX
    IO::GPIO* muxSelectArr[3] = {
        &IO::getGPIO<BMS::BMS::MUX_S1_PIN>(),
        &IO::getGPIO<BMS::BMS::MUX_S2_PIN>(),
        &IO::getGPIO<BMS::BMS::MUX_S3_PIN>(),
    };
    IO::ADC& thermAdc = IO::getADC<BMS::BMS::TEMP_INPUT_PIN>();

    BMS::dev::ThermistorMux thermMux(muxSelectArr, thermAdc);

    DEV::IWDG& iwdg = DEV::getIWDG(500);

    // Initialize the BMS itself
    BMS::BMS bms(bqSettingsStorage, bq, interlock, alarm, systemDetect, bmsOK, errorLed, thermMux, resetHandler, iwdg);

    ///////////////////////////////////////////////////////////////////////////
    // Setup CAN configuration, this handles making drivers, applying settings.
    // And generally creating the CANopen stack node which is the interface
    // between the application (the code we write) and the physical CAN network
    ///////////////////////////////////////////////////////////////////////////

    // Reserved memory for CANopen stack usage
    uint8_t sdoBuffer[CO_SSDO_N * CO_SDO_BUF_BYTE];
    CO_TMR_MEM appTmrMem[16];

    // Reserve driver variables
    CO_IF_DRV canStackDriver;

    CO_IF_CAN_DRV canDriver;
    CO_IF_TIMER_DRV timerDriver;
    CO_IF_NVM_DRV nvmDriver;

    CO_NODE canNode;

    // Attempt to join the CAN network
    IO::CAN::CANStatus result = can.connect();

    // test that the board is connected to the can network
    if (result != IO::CAN::CANStatus::OK) {
        log::LOGGER.log(log::Logger::LogLevel::ERROR, "Failed to connect to CAN network\r\n");
        return 1;
    }

    // Initialize all the CANOpen drivers.
    IO::initializeCANopenDriver(&canOpenQueue, &can, &timer, &canStackDriver, &nvmDriver, &timerDriver, &canDriver);

    // Initialize the CANOpen node we are using.
    IO::initializeCANopenNode(&canNode, &bms, &canStackDriver, sdoBuffer, appTmrMem);

    // Set the node to operational mode
    CONmtSetMode(&canNode.Nmt, CO_OPERATIONAL);

    time::wait(500);

    if (CO_ERR canError = CONodeGetErr(&canNode); canError != CO_ERR_NONE) {
        log::LOGGER.log(log::Logger::LogLevel::INFO, "CANopen initialization failed %d", canError);
        return 1;
    }

    log::LOGGER.log(log::Logger::LogLevel::INFO, "CAN initialization complete");

    // Main processing loop, contains the following logic
    // 1. Update CANopen logic and processing incoming messages
    // 2. Run per-loop BMS state logic
    // 3. Wait for new data to come in
    while (1) {
        // Process CANopen
        IO::processCANopenNode(&canNode);
        // Update the state of the BMS
        bms.process();
        // Wait for new data to come in
        time::wait(10);
    }
}
