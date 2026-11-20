/**
 * This test demonstrates the functionality of the ThermistorMux class.
 */

#include <core/io/ADC.hpp>
#include <core/io/GPIO.hpp>
#include <core/manager.hpp>
#include <core/utils/time.hpp>

#include <BMS.hpp>
#include <dev/ThermistorMux.hpp>

namespace io = core::io;

namespace time = core::time;

int main() {
    core::platform::init();

    io::ADC& adc = io::getADC<BMS::BMS::TEMP_INPUT_PIN>();
    io::UART& uart = io::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(115200);

    io::GPIO& muxs1 = io::getGPIO<BMS::BMS::MUX_S1_PIN>();
    io::GPIO& muxs2 = io::getGPIO<BMS::BMS::MUX_S2_PIN>();
    io::GPIO& muxs3 = io::getGPIO<BMS::BMS::MUX_S3_PIN>();

    io::GPIO* muxPinArr[3] = {&muxs1, &muxs2, &muxs3};

    BMS::dev::ThermistorMux thermistorMux(muxPinArr, adc);

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
