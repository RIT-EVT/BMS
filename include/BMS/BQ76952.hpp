#ifndef _BMS_BQ76952_
#define _BMS_BQ76952_


#include <EVT/io/I2C.hpp>

#include <BMS/BQSetting.hpp>

#include <stdint.h>

namespace BMS {

class BQ76952 {
public:

    /**
     * Create a new instance of the BQ76952 chip.
     *
     * @param i2c[in] I2C interface used to communicate with the BQ76952
     * @param address[in] The I2C address of the BQ76952
     */
    BQ76952(EVT::core::IO::I2C& i2c, uint8_t address);

    /**
     * Send a single setting setting over to the BQ76952
     *
     * @param setting[in] The setting to send to the BQ76952
     */
    void sendSetting(BQSetting& setting);

private:
    static constexpr uint8_t RAM_TRANSFER_LOCATION = 0x3E;

    /**
     * Used for communicating with the BQ chip
     */
    EVT::core::IO::I2C& i2c;
    /**
     * The I2C address of the BQ76952
     */
    uint8_t address;
};

}  // namespace BMS

#endif
