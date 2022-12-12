/**
 * This is a basic sample of using the UART module. The program provides a
 * basic echo functionality where the uart will write back whatever the user
 * enters.
 */
#include <EVT/io/CANopen.hpp>
#include <EVT/io/UART.hpp>
#include <EVT/io/manager.hpp>
#include <EVT/io/pin.hpp>
#include <EVT/io/types/CANMessage.hpp>

#include <EVT/dev/platform/f3xx/f302x8/Timerf302x8.hpp>
#include <EVT/dev/storage/EEPROM.hpp>
#include <EVT/dev/storage/M24C32.hpp>

#include <EVT/utils/types/FixedQueue.hpp>

#include <BMS/BMS.hpp>
#include <BMS/BMSLogger.hpp>
#include <BMS/dev/BQ76952.hpp>
#include <BMS/dev/SystemDetect.hpp>

namespace IO = EVT::core::IO;
namespace DEV = EVT::core::DEV;
namespace time = EVT::core::time;

#define BIKE_HEART_BEAT 0x715
#define CHARGER_HEART_BEAT 0x716
#define TIMEOUT 1000

/**
 * Interrupt handler for incoming CAN messages.
 *
 * This will just update the state of the system detect for testing
 * purposes
 */
void canInterruptHandler(IO::CANMessage& message, void* priv) {
    BMS::DEV::SystemDetect* systemDetect = (BMS::DEV::SystemDetect*) priv;

    systemDetect->processHeartBeat(message.getId());
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
    IO::init();

    // Initialize the system detect
    BMS::DEV::SystemDetect systemDetect(BIKE_HEART_BEAT, CHARGER_HEART_BEAT, TIMEOUT);

    // Initialize IO
    IO::CAN& can = IO::getCAN<IO::Pin::PA_12, IO::Pin::PA_11>();
    can.addIRQHandler(canInterruptHandler, reinterpret_cast<void*>(&systemDetect));
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);
    EVT::core::IO::I2C& i2c = EVT::core::IO::getI2C<IO::Pin::PB_8, IO::Pin::PB_9>();

    // Initialize the timer
    DEV::Timerf302x8 timer(TIM2, 100);

    // Initialize the EEPROM
    EVT::core::DEV::M24C32 eeprom(0x50, i2c);

    // Intialize the logger
    BMS::LOGGER.setUART(&uart);
    BMS::LOGGER.setLogLevel(BMS::BMSLogger::LogLevel::ERROR);

    // Initialize the BQ interfaces
    BMS::DEV::BQ76952 bq(i2c, 0x08);
    BMS::BQSettingsStorage bqSettingsStorage(eeprom, bq);

    // Intialize the Interlock
    // TODO: Determine actual interlock GPIO
    IO::GPIO& interlockGPIO = IO::getGPIO<IO::Pin::PB_0>(IO::GPIO::Direction::INPUT);
    BMS::DEV::Interlock interlock(interlockGPIO);

    // Intialize the alarm pin
    IO::GPIO& alarm = IO::getGPIO<IO::Pin::PB_1>(IO::GPIO::Direction::INPUT);

    // Initialize the system OK pin
    // TODO: Determine actual system ok pin
    //IO::GPIO& bmsOK = IO::getGPIO<IO::Pin::PC_14>(IO::GPIO::Direction::OUTPUT);

    // Intialize the BMS itself
    BMS::BMS bms(bqSettingsStorage, bq, interlock, alarm, systemDetect/*, bmsOK*/);

    // Reserved memory for CANopen stack usage
    uint8_t sdoBuffer[1][CO_SDO_BUF_BYTE];
    CO_TMR_MEM appTmrMem[4];

    // Queue that will store CANopen messages
    EVT::core::types::FixedQueue<CANOPEN_QUEUE_SIZE, IO::CANMessage> canOpenQueue;

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

    // Intialize CANopen logic
    CONodeInit(&canNode, &canSpec);
    CONodeStart(&canNode);
    CONmtSetMode(&canNode.Nmt, CO_OPERATIONAL);

    // Main processing loop, contains the following logic
    // 1. Update CANopen logic and processing incomming messages
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
