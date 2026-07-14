/**
* This is a target used for testing BMS CANopen output
*/

#include <core/io/CANopen.hpp>
#include <core/io/UART.hpp>
#include <core/io/pin.hpp>
#include <core/io/types/CANMessage.hpp>
#include <core/manager.hpp>

#include <core/dev/storage/EEPROM.hpp>
#include <core/dev/storage/M24C32.hpp>

#include <core/utils/log.hpp>
#include <core/utils/types/FixedQueue.hpp>

#include "SystemDetect.hpp"
#include <BMS.hpp>
#include <dev/BQ76952.hpp>

namespace io = core::io;
namespace dev = core::dev;
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
    core::types::FixedQueue<CANOPEN_QUEUE_SIZE, io::CANMessage>* queue;
    BMS::SystemDetect* systemDetect;
};

/**
* Interrupt handler for incoming CAN messages.
*
* @param priv[in] The private data (FixedQueue<CANOPEN_QUEUE_SIZE, CANMessage>)
*/
void canInterruptHandler(io::CANMessage& message, void* priv) {
    struct CANInterruptParams* params = (CANInterruptParams*) priv;

    core::types::FixedQueue<CANOPEN_QUEUE_SIZE, io::CANMessage>* queue =
        params->queue;
    BMS::SystemDetect* systemDetect = params->systemDetect;

    systemDetect->processHeartbeat(message.getId());

    if (queue == nullptr)
        return;
    if (!message.isCANExtended())
        queue->append(message);
}

///////////////////////////////////////////////////////////////////////////////
// CANopen specific Callbacks. Need to be defined in some location
///////////////////////////////////////////////////////////////////////////////
extern "C" void CONodeFatalError(void) {
    log::LOGGER.log(log::Logger::LogLevel::ERROR, "Fatal CANopen error");
}

int main() {
    // Initialize system
    core::platform::init();

    // Queue that will store CANopen messages
    core::types::FixedQueue<CANOPEN_QUEUE_SIZE, io::CANMessage> canOpenQueue;

    // Initialize the system detect
    BMS::SystemDetect systemDetect(BIKE_HEART_BEAT, CHARGER_HEART_BEAT,
                                   DETECT_TIMEOUT);

    BMS::ResetHandler resetHandler;

    // Create struct that will hold CAN interrupt parameters
    struct CANInterruptParams canParams = {
        .queue = &canOpenQueue,
        .systemDetect = &systemDetect,
    };

    // Initialize io
    io::CAN& can = io::getCAN<BMS::BMS::CAN_TX_PIN, BMS::BMS::CAN_RX_PIN>();
    can.addIRQHandler(canInterruptHandler, reinterpret_cast<void*>(&canParams));
    io::UART& uart = io::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(115200);
    io::I2C& i2c = io::getI2C<BMS::BMS::I2C_SCL_PIN, BMS::BMS::I2C_SDA_PIN>();

    // Initialize the timer
    dev::Timer& timer = dev::getTimer<dev::MCUTimer::Timer2>(100);

    // Initialize the EEPROM
    core::dev::M24C32 eeprom(0x57, i2c);

    // Initialize the logger
    log::LOGGER.setUART(&uart);
    log::LOGGER.setLogLevel(log::Logger::LogLevel::DEBUG);

    // Initialize the BQ interfaces
    io::GPIO& bqReset = io::getGPIO<BMS::BMS::BQ_RESET_PIN>();
    BMS::dev::BQ76952 bq(i2c, 0x08, bqReset);
    BMS::BQSettingsStorage bqSettingsStorage(eeprom, bq);

    // Initialize the Interlock
    io::GPIO& interlockGPIO = io::getGPIO<BMS::BMS::INTERLOCK_PIN>(io::GPIO::Direction::INPUT);
    BMS::dev::Interlock interlock(interlockGPIO);

    // Initialize the alarm pin
    io::GPIO& alarm = io::getGPIO<BMS::BMS::ALARM_PIN>(io::GPIO::Direction::INPUT);

    // Initialize the system OK pin
    io::GPIO& bmsOK = io::getGPIO<BMS::BMS::OK_PIN>(io::GPIO::Direction::OUTPUT);

    // Initialize the error LED pin
    io::GPIO& errorLed = io::getGPIO<BMS::BMS::ERROR_LED_PIN>(io::GPIO::Direction::OUTPUT);

    // Initialize the thermistor MUX
    io::GPIO* muxSelectArr[3] = {
        &io::getGPIO<BMS::BMS::MUX_S1_PIN>(),
        &io::getGPIO<BMS::BMS::MUX_S2_PIN>(),
        &io::getGPIO<BMS::BMS::MUX_S3_PIN>(),
    };
    io::ADC& thermAdc = io::getADC<BMS::BMS::TEMP_INPUT_PIN>();

    BMS::dev::ThermistorMux thermMux(muxSelectArr, thermAdc);

    dev::IWDG& iwdg = dev::getIWDG(500);

    // Initialize the BMS itself
    BMS::BMS bms(bqSettingsStorage, bq, interlock, alarm, systemDetect, bmsOK, errorLed, thermMux, resetHandler, iwdg);

    ///////////////////////////////////////////////////////////////////////////
    // Setup CAN configuration, this handles making drivers, applying settings.
    // And generally creating the CANopen stack node which is the interface
    // between the application (the code we write) and the physical CAN network
    ///////////////////////////////////////////////////////////////////////////

    // Will store CANopen messages that will be populated by the EVT-core CAN
    // interrupt

    // Reserved memory for CANopen stack usage
    uint8_t sdoBuffer[CO_SSDO_N * CO_SDO_BUF_BYTE];
    CO_TMR_MEM appTmrMem[16];

    // Initialize the CANopen drivers
    CO_IF_DRV canStackDriver;
    CO_IF_CAN_DRV canDriver;
    CO_IF_TIMER_DRV timerDriver;
    CO_IF_NVM_DRV nvmDriver;
    CO_NODE canNode;

    // Initialize all the CANOpen drivers.
    io::initializeCANopenDriver(&canOpenQueue, &can, &timer, &canStackDriver, &nvmDriver, &timerDriver, &canDriver);

    // Initialize the CANOpen node we are using.
    io::initializeCANopenNode(&canNode, &bms, &canStackDriver, sdoBuffer, appTmrMem);
    time::wait(500);

    // Attempt to join the CAN network
    io::CAN::CANStatus result = can.connect(true);

    if (result != io::CAN::CANStatus::OK) {
        uart.printf("Failed to connect to CAN network\r\n");
        return 1;
    }

    CONmtSetMode(&canNode.Nmt, CO_OPERATIONAL);

    log::LOGGER.log(log::Logger::LogLevel::INFO, "Initialization complete");

    // Main processing loop, contains the following logic
    // 1. Update CANopen logic and processing incoming messages
    // 2. Run per-loop BMS state logic
    // 3. Wait for new data to come in
    while (1) {
        // Process CANopen
        io::processCANopenNode(&canNode);
        // Update the state of the BMS
        bms.canTest();
        // Wait for new data to come in
        time::wait(10);
    }
}
