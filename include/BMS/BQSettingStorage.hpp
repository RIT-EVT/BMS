#pragma once

#include <stdint.h>

#include <EVT/dev/storage/M24C32.hpp>

#include <BMS/BQSetting.hpp>
#include <BMS/dev/BQ76952.hpp>

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
    BQSettingsStorage(EVT::core::DEV::M24C32& eeprom, DEV::BQ76952& bq);

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
     * Read in a setting from EEPROM and update the provided setting. This
     * will update the location in memory to read the next setting from.
     * Therefore, users can read sequential setting by simply calling this
     * method multiple times
     *
     * @param setting[out] The setting to update from EEPROM
     */
    void readSetting(BQSetting& setting);

    /**
     * Write out the given setting into EEPROM. Will write out the data using
     * the eepromOffset value. This will update the location to memory to
     * write the next setting to. Therefore, users can write sequential
     * settings by simply calling this method multiple times.
     *
     * @param setting[in] The setting to write to EEPROM
     */
    void writeSetting(BQSetting& setting);

    /**
     * Write the number of settings into EEPROM.
     */
    void writeNumSettings();

    /**
     * Reset the EEPROM offset where to write setting back to the being
     */
    void resetEEPROMOffset();

    /**
     * Get the EEPROM instance, used for CANopen reading and writting into
     * memory.
     *
     * @return The EEPROM instance
     */
    EVT::core::DEV::M24C32& getEEPROM();

    /**
     * Transfer settings from EEPROM to the BQ chip. This will read the
     * settings one-by-one and pass the setting to the BQ chip for programming.
     */
    BMS::DEV::BQ76952::Status transferSettings();

private:
    /**
     * The starting address in EEPROM where the BQ settings are stored.
     */
    uint32_t startAddress;
    /**
     * Keeps track of the address in memory to write to
     */
    uint32_t addressLocation;
    /**
     * The number of settings that are being stored for the BQ.
     */
    uint16_t numSettings;
    /**
     * CANopen stack interface. Exposes the BQ settings over CANopen
     */
    CO_OBJ_TYPE canOpenInterface;
    /**
     * EEPROM for storing the BQ settings.
     */
    EVT::core::DEV::M24C32& eeprom;
    /**
     * The BQ chip interface
     */
    DEV::BQ76952& bq;

    friend class BMS;
};

}// namespace BMS
