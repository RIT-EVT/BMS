/**
 * This is a basic sample of using the UART module. The program provides a
 * basic echo functionality where the uart will write back whatever the user
 * enters.
 */

#include <EVT/io/CANopen.hpp>
#include <EVT/io/UART.hpp>
#include <EVT/io/pin.hpp>
#include <EVT/io/types/CANMessage.hpp>
#include <EVT/manager.hpp>

#include <EVT/dev/storage/EEPROM.hpp>
#include <EVT/dev/storage/M24C32.hpp>

#include <EVT/utils/log.hpp>
#include <EVT/utils/types/FixedQueue.hpp>

#include "SystemDetect.hpp"
#include <BMS.hpp>
#include <dev/BQ76952.hpp>

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;
namespace time = EVT::core::time;
namespace log = EVT::core::log;

#define BIKE_HEART_BEAT 0x70A   // NODE_ID = 10
#define CHARGER_HEART_BEAT 0x710// NODE_ID = 16
#define DETECT_TIMEOUT 1000

/**
* This struct is a catchall for data that is needed by the CAN interrupt
* handler. An instance of this struct will be provided as the parameter
* to the interrupt handler.
*/
struct CANInterruptParams {
    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage>* queue;
    BMS::SystemDetect* systemDetect;
};

/**
* Interrupt handler for incoming CAN messages.
*
* @param priv[in] The private data (FixedQueue<CANOPEN_QUEUE_SIZE, CANMessage>)
*/
void canInterruptHandler(IO::CANMessage& message, void* priv) {
    struct CANInterruptParams* params = (CANInterruptParams*) priv;

    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage>* queue =
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
}

extern "C" void COIfCanReceive(CO_IF_FRM* frm) {}

extern "C" int16_t COLssStore(uint32_t baudrate, uint8_t nodeId) { return 0; }

extern "C" int16_t COLssLoad(uint32_t* baudrate, uint8_t* nodeId) { return 0; }

extern "C" void CONmtModeChange(CO_NMT* nmt, CO_MODE mode) {}

extern "C" void CONmtHbConsEvent(CO_NMT* nmt, uint8_t nodeId) {}

extern "C" void CONmtHbConsChange(CO_NMT* nmt, uint8_t nodeId, CO_MODE mode) {}

extern "C" int16_t COParaDefault(CO_PARA* pg) { return 0; }

extern "C" void COPdoTransmit(CO_IF_FRM* frm) {}

extern "C" int16_t COPdoReceive(CO_IF_FRM* frm) { return 0; }

extern "C" void COPdoSyncUpdate(CO_RPDO* pdo) {}

extern "C" void COTmrLock(void) {}

extern "C" void COTmrUnlock(void) {}

int main() {
    // Initialize system
    EVT::core::platform::init();

    // Queue that will store CANopen messages
    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage> canOpenQueue;

    // Initialize the system detect
    BMS::SystemDetect systemDetect(BIKE_HEART_BEAT, CHARGER_HEART_BEAT,
                                   DETECT_TIMEOUT);

    BMS::ResetHandler resetHandler;

    // Create struct that will hold CAN interrupt parameters
    struct CANInterruptParams canParams = {
        .queue = &canOpenQueue,
        .systemDetect = &systemDetect,
    };

    // Initialize IO
    IO::CAN& can = IO::getCAN<BMS::BMS::CAN_TX_PIN, BMS::BMS::CAN_RX_PIN>();
    can.addIRQHandler(canInterruptHandler, reinterpret_cast<void*>(&canParams));
    IO::UART& uart = IO::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(115200, true);
    IO::I2C& i2c = IO::getI2C<BMS::BMS::I2C_SCL_PIN, BMS::BMS::I2C_SDA_PIN>();

    // Initialize the timer
    DEV::Timer& timer = DEV::getTimer<DEV::MCUTimer::Timer2>(100);

    // Initialize the EEPROM
    EVT::core::DEV::M24C32 eeprom(0x50, i2c);

    // Initialize the logger
    log::LOGGER.setUART(&uart);
    log::LOGGER.setLogLevel(log::Logger::LogLevel::ERROR);

    // Initialize the BQ interfaces
    BMS::DEV::BQ76952 bq(i2c, 0x08);
    BMS::BQSettingsStorage bqSettingsStorage(eeprom, bq);

    // Initialize the Interlock
    // TODO: Determine actual interlock GPIO
    IO::GPIO& interlockGPIO = IO::getGPIO<BMS::BMS::INTERLOCK_PIN>(IO::GPIO::Direction::INPUT);
    BMS::DEV::Interlock interlock(interlockGPIO);

    // Initialize the alarm pin
    IO::GPIO& alarm = IO::getGPIO<BMS::BMS::ALARM_PIN>(IO::GPIO::Direction::INPUT);

    // Initialize the system OK pin
    // TODO: Determine actual system ok pin
    IO::GPIO& bmsOK = IO::getGPIO<BMS::BMS::OK_PIN>(IO::GPIO::Direction::OUTPUT);

    // Initialize the thermistor MUX
    IO::GPIO* muxSelectArr[3] = {
        &IO::getGPIO<BMS::BMS::MUX_S1_PIN>(),
        &IO::getGPIO<BMS::BMS::MUX_S2_PIN>(),
        &IO::getGPIO<BMS::BMS::MUX_S3_PIN>(),
    };
    IO::ADC& thermAdc = IO::getADC<BMS::BMS::TEMP_INPUT_PIN>();

    BMS::DEV::ThermistorMux thermMux(muxSelectArr, thermAdc);

    DEV::IWDG& iwdg = DEV::getIWDG(500);

    // Initialize the BMS itself
    BMS::BMS bms(bqSettingsStorage, bq, interlock, alarm, systemDetect, bmsOK, thermMux, resetHandler, iwdg);

    // Reserved memory for CANopen stack usage
    uint8_t sdoBuffer[1][CO_SDO_BUF_BYTE];
    CO_TMR_MEM appTmrMem[4];

    // Initialize the CANopen drivers
    CO_IF_DRV canStackDriver;
    CO_IF_CAN_DRV canDriver;
    CO_IF_TIMER_DRV timerDriver;
    CO_IF_NVM_DRV nvmDriver;
    IO::getCANopenCANDriver(&can, &canOpenQueue, &canDriver);
    IO::getCANopenTimerDriver(&timer, &timerDriver);
    IO::getCANopenNVMDriver(&nvmDriver);

    // Attach the CANopen drivers
    canStackDriver.Can = &canDriver;
    canStackDriver.Timer = &timerDriver;
    canStackDriver.Nvm = &nvmDriver;

    CO_NODE_SPEC canSpec = {
        .NodeId = BMS::BMS::NODE_ID,
        .Baudrate = IO::CAN::DEFAULT_BAUD,
        .Dict = bms.getObjectDictionary(),
        .DictLen = bms.getObjectDictionarySize(),
        .EmcyCode = NULL,
        .TmrMem = appTmrMem,
        .TmrNum = 16,
        .TmrFreq = 100,
        .Drv = &canStackDriver,
        .SdoBuf = reinterpret_cast<uint8_t*>(&sdoBuffer[0]),
    };

    CO_NODE canNode;
    time::wait(500);

    // Join the CANopen network
    can.connect();

    // Initialize CANopen logic
    CONodeInit(&canNode, &canSpec);
    CONodeStart(&canNode);
    CONmtSetMode(&canNode.Nmt, CO_OPERATIONAL);

    // Main processing loop, contains the following logic
    // 1. Update CANopen logic and processing incoming messages
    // 2. Run per-loop BMS state logic
    // 3. Wait for new data to come in
    while (1) {
        // Process incoming CAN messages
        CONodeProcess(&canNode);
        // Update the state of timer based events
        COTmrService(&canNode.Tmr);
        // Handle executing timer events that have elapsed
        COTmrProcess(&canNode.Tmr);

        uart.printf("Detected System: %d\r\n", systemDetect.getIdentifiedSystem());

        // Wait for new data to come in
        time::wait(100);
    }
}
