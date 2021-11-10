#ifndef _BQ_SETTING_
#define _BQ_SETTING_

#include <stdint.h>

namespace BMS {

/**
 * Represents the settings that are sent out to the BQ chip. Settings are
 * made to three potential areas of the BQ chup.
 *
 * 1. Direct: Settings applied directly via an I2C address.
 *      ex) Register 0xC4 is given to 0x56
 * 2. Subcommand: The subcommand address is written out to two I2C registers
 *      of the BQ chip and the value is written out to the 4 value register.
 * 3. RAM: Similar to subcommand
 */
class BQSetting {
public:
    /**
     * The size of the array in which the BQSetting can be fit into.
     * Units are bytes.
     */
    static constexpr uint8_t ARRAY_SIZE = 7;

    /**
     * Represents the different options where the settings can be applied.
     */
    enum class BQSettingType {
        DIRECT = 0u,
        SUBCOMMAND = 1u,
        RAM = 2u
    };

    /**
     * Populate the content of the object with the values parsed from the
     * provided array. The array is in the format below.
     *
     * Byte 0:
     *      Bit 0 and Bit 1: Command type
     *          00 -> Direct
     *          01 -> Subcommand
     *          10 -> RAM
     *          11 -> Unused
     *      Bit 2-4: Number of bytes of data
     *          Direct -> Always 1
     *          Subcommand -> Either 4 or 0
     *          RAM -> Either 4 or 0
     * Byte 1-2: Address to write to of the BQ chip
     * Byte 3-6: Data
     *
     * @param buffer[in] The array to pull data from
     */
    void fromArray(uint8_t buffer[ARRAY_SIZE]);

    /**
     * Popuate an array with the bit representation of the BQSettings.
     * This follows the form shown on BQSetting::fromArray.
     *
     * @param buffer[out] The array to populate with parsed data
     */
    void toArray(uint8_t buffer[ARRAY_SIZE]);

    /**
     * Get the setting type.
     *
     * @return The setting type
     */
    BQSettingType getSettingType();

    /**
     * Get the address. For direct commands the address is 8 bits in size,
     * for subcommand and RAM it is 16 bits.
     *
     * @return The address of the setting
     */
    uint16_t getAddress();

    /**
     * Get the contained data as a 32 bit value.
     *
     * @return The data as a 32 bit value
     */
    uint32_t getData();

private:
    /** The type of the command */
    BQSettingType settingType;
    /**
     * The address to write to for the setting. When the setting type is
     * direct, the address is 8 bits. Otherwise it is 16 bits.
     */
    uint16_t address;
    /**
     * The data for the setting. When the setting type is direct, the value
     * is 8 bits in size. Otherwise it can be up to 32 bits in size.
     */
    uint32_t data;
    /**
     * Number of bytes of data associated with the settingm can be 0
     */
    uint32_t numBytes;
};

}  // namespace BMS

#endif
