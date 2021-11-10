#ifndef _BQ_SETTING_STORAGE_
#define _BQ_SETTING_STORAGE_

#include <stdint.h>

#include <EVT/dev/storage/platform/M24C32.hpp>

#include <BMS/BQSetting.hpp>

#include <Canopen/co_obj.h>

namespace BMS {

/**
 * Handles the storage logic of the BQ settings. Storage takes place in
 * EEPROM.
 *
 * Part of the logic for the BQ storage handler is exposing the settings
 * over the CANopen network. This is handled by producing a CANopen stack
 * driver for a custom field. That field has the ability to read and write
 * BQ settings through the storage handler. The value
 * `BQSettingsStorage::canOpenInterface` can be added to the CANopen
 * object dictionary to allow settings to be sent over. The process to write
 * out the settings should be
 *
 *  1) Update the number of settings that are stored. The number of settings
 *     will be exposed over CANopen (`BQSettingsStorage::numSettings`).
 *  2) Using SDO segmented download, send over each BQ Setting
 *
 * The settings will be written into EEPROM. Once all settings have come
 * over CANopen, the number of settings in EEPROM will also be updated
 * accordingly.
 */
class BQSettingsStorage {
public:

    BQSettingsStorage(EVT::core::DEV::M24C32& eeprom);

    /**
     * Get the number of settings stored for the BQ
     *
     * @return The number of settings stored
     */
    uint32_t getNumSettings();

    /**
     * Set the number of settings that are stored for the BQ chip. This
     * will update the value stored in EEPROM.
     *
     * NOTE: This should only be done after the correct number of settings
     * have been written out. Updating the settings to an incorrect value
     * then attempting to read in the settings can cause unexpected results.
     *
     * @param numSettings[in] The new number of settings
     */
    void setNumSettings(uint32_t numSettings);

    /**
     * Read in a setting from EEPROM and update the provided setting.
     *
     * @param index[in] The index of the setting to read in (0 <= index < numSettings)
     * @param setting[out] The setting to update from EEPROM
     */
    void readSetting(uint32_t index, BQSetting& setting);

    /**
     * Write out the given setting into EEPROM.
     *
     * @param index[in] The index of the setting to write out (0 <= index < numSettings)
     * @param setting[in] The setting to write to EEPROM
     */
    void writeSetting(uint32_t index, BQSetting& setting);

    /**
     * Reset the EEPROM offset back to zero.
     */
    void resetEEPROMOffset();

    /**
     * Increment the EEPROM offset (measured in number of BQSettings)
     */
    void incrementEEPROMOffset();

private:
    /**
     * The starting address in EEPROM where the BQ settings are stored.
     */
    uint32_t startAddress;
    /**
     * The number of settings that are being stored for the BQ.
     */
    uint8_t numSettings;
    /**
     * The offset into the EEPROM to write to (measured in numbers of settings)
     */
    uint8_t eepromOffset;
    /**
     * CANopen stack interface. Exposes the BQ settings over CANopen
     */
    CO_OBJ_TYPE canOpenInterface;
    /**
     * EEPROM for storing the BQ settings.
     */
    EVT::core::DEV::M24C32& eeprom;

    friend class BMS;
};


}  // namespace BMS

#endif
