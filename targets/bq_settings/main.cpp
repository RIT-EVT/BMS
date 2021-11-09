#include <EVT/io/manager.hpp>
#include <BMS/BMS.hpp>
#include <BMS/BQSettingStorage.hpp>
#include <BMS/BMSLogger.hpp>

namespace IO = EVT::core::IO;

///////////////////////////////////////////////////////////////////////////////
// CANopen specific Callbacks. Need to be defined in some location
///////////////////////////////////////////////////////////////////////////////
extern "C" void CONodeFatalError(void) { }

extern "C" void COIfCanReceive(CO_IF_FRM *frm) { }

extern "C" int16_t COLssStore(uint32_t baudrate, uint8_t nodeId) { return 0; }

extern "C" int16_t COLssLoad(uint32_t *baudrate, uint8_t *nodeId) { return 0; }

extern "C" void CONmtModeChange(CO_NMT *nmt, CO_MODE mode) { }

extern "C" void CONmtHbConsEvent(CO_NMT *nmt, uint8_t nodeId) { }

extern "C" void CONmtHbConsChange(CO_NMT *nmt, uint8_t nodeId, CO_MODE mode) { }

extern "C" int16_t COParaDefault(CO_PARA *pg) { return 0; }

extern "C" void COPdoTransmit(CO_IF_FRM *frm) { }

extern "C" int16_t COPdoReceive(CO_IF_FRM *frm) { return 0; }

extern "C" void COPdoSyncUpdate(CO_RPDO *pdo) { }

extern "C" void COTmrLock(void) { }

extern "C" void COTmrUnlock(void) { }

int main() {
    IO::UART& uart = IO::getUART<IO::Pin::UART_TX, IO::Pin::UART_RX>(9600);
    BMS::LOGGER.setUART(&uart);

    // Instance of the setting repository
    // BMS::BQSettingsStorage bqSettingsStorage;

    // Instance of the BMS
    // BMS::BMS bms(bqSettingsStorage);

    // Have the CANopen interface run
    while (1) {
        // Process incoming CAN messages
    }
}
