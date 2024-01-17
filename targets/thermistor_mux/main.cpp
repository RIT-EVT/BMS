/**
* Tests the
*/

#include <EVT/io/ADC.hpp>
#include <EVT/io/GPIO.hpp>
#include <EVT/manager.hpp>
#include <EVT/utils/time.hpp>

#include <BMS.hpp>
#include <dev/ThermistorMux.hpp>

namespace IO = EVT::core::IO;

namespace time = EVT::core::time;

int main() {
    EVT::core::platform::init();

    IO::ADC& adc = IO::getADC<BMS::BMS::TEMP_INPUT_PIN>();
    IO::UART& uart = IO::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(115200, true);

    IO::GPIO& muxs1 = IO::getGPIO<BMS::BMS::MUX_S1_PIN>();
    IO::GPIO& muxs2 = IO::getGPIO<BMS::BMS::MUX_S2_PIN>();
    IO::GPIO& muxs3 = IO::getGPIO<BMS::BMS::MUX_S3_PIN>();

    IO::GPIO* muxPinArr[3] = {&muxs1, &muxs2, &muxs3};

    BMS::DEV::ThermistorMux thermistorMux(muxPinArr, adc);

    uint8_t looper = 0;

    uart.printf("Starting Thermistor Mux Testing -----\r\n");

    time::wait(500);

    while (1) {
        uart.printf("%d\r\n", thermistorMux.getTemp(looper));

        looper = (looper + 1) % 8;
        if (looper == 0) {
            uart.printf("------------------------------------------------------------------------\r\n");
            time::wait(1000);
        }
    }
}
