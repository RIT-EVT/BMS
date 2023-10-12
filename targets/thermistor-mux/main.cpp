/**
* Tests the
*/

#include <EVT/dev/Thermistor.hpp>
#include <EVT/io/ADC.hpp>
#include <EVT/io/GPIO.hpp>
#include <EVT/manager.hpp>
#include <EVT/utils/time.hpp>

#include <BMS.hpp>
#include <dev/ThermistorMux.hpp>

namespace IO = EVT::core::IO;

namespace time = EVT::core::time;

void switchMux(IO::GPIO* muxArr[3], uint8_t inputNum) {
    for (uint8_t i = 0; i < 3; i++) {
        muxArr[i]->writePin(inputNum & 1 << i ? IO::GPIO::State::HIGH : IO::GPIO::State::LOW);
    }
}

uint32_t convert(uint32_t voltage) {
    return voltage;
}

int main() {
    EVT::core::platform::init();

    IO::ADC& adc = IO::getADC<BMS::BMS::TEMP_INPUT_PIN>();
    IO::UART& uart = IO::getUART<BMS::BMS::UART_TX_PIN, BMS::BMS::UART_RX_PIN>(115200, true);

    IO::GPIO& muxs1 = IO::getGPIO<BMS::BMS::MUX_S1_PIN>();
    IO::GPIO& muxs2 = IO::getGPIO<BMS::BMS::MUX_S2_PIN>();
    IO::GPIO& muxs3 = IO::getGPIO<BMS::BMS::MUX_S3_PIN>();

    IO::GPIO* muxPinArr[3] = {&muxs1, &muxs2, &muxs3};

    BMS::DEV::ThermistorMux tmux(muxPinArr, adc);

    uint8_t looper = 0;

    uart.printf("Starting ADC Voltage Testing -----\r\n");

    time::wait(500);

    while (1) {
        uart.printf("%d\r\n", tmux.getTemp(looper));

        looper = (looper + 1) % 8;
        if (looper == 0) {
            uart.printf("------------------------------------------------------------------------\r\n");
            time::wait(1000);
        }
    }
}
