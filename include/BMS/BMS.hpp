#include <stdint.h>
#include <Canopen/co_core.h>
#include <BMS/BQSettingStorage.hpp>

namespace BMS {

/**
 * Interface to the BMS board. Includes the CANopen object dictionary that
 * defined the features that are exposed by the BMS on the CANopen network.
 */
class BMS {
public:
    BMS(BQSettingsStorage& bqSettingsStorage);

    /**
     * The node ID used to identify the device on the CAN network.
     */
    static constexpr uint8_t NODE_ID = 0x05;

    /**
     * Get a pointer to the start of the CANopen object dictionary.
     *
     * @return Pointer to the start of the CANopen object dictionary.
     */
    CO_OBJ_T* getObjectDictionary();

    /**
     * Get the number of elements in the object dictionary.
     *
     * @return The number of elements in the object dictionary
     */
    uint16_t getObjectDictionarySize();

private:
    /**
     * Have to know the size of the object dictionary for initialization
     * process.
     */
    static constexpr uint16_t OBJECT_DIRECTIONARY_SIZE = 16;

    /**
     * The interface for storaging and retrieving BQ Settings.
     */
    BQSettingsStorage& bqSettingsStorage;

    /**
     * The object dictionary of the BMS. Includes settings that determine
     * how the BMS functions on the CANopen network as well as the data
     * that is exposed on the network.
     *
     * Array of CANopen objects. +1 for the special "end-of-array" marker
     */
    CO_OBJ_T objectDictionary[OBJECT_DIRECTIONARY_SIZE + 1] = {
        // Sync ID, defaults to 0x80
        { CO_KEY(0x1005, 0, CO_UNSIGNED32|CO_OBJ_D__R_), 0, (uintptr_t)0x80 },

        // Information about the hardware, hard coded sample values for now
        // 1: Vendor ID
        // 2: Product Code
        // 3: Revision Number
        // 4: Serial Number
        {
            .Key = CO_KEY(0x1018, 1, CO_UNSIGNED32|CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t)0x10
        },
        {
            .Key = CO_KEY(0x1018, 2, CO_UNSIGNED32|CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t)0x11
        },
        {
            .Key = CO_KEY(0x1018, 3, CO_UNSIGNED32|CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t)0x12
        },
        {
            .Key = CO_KEY(0x1018, 4, CO_UNSIGNED32|CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t)0x13
        },

        // SDO CAN message IDS.
        // 1: Client -> Server ID, default is 0x600 + NODE_ID
        // 2: Server -> Client ID, default is 0x580 + NODE_ID
        {
            .Key = CO_KEY(0x1200, 1, CO_UNSIGNED32|CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t)0x600 + NODE_ID
        },
        {
            .Key = CO_KEY(0x1200, 2, CO_UNSIGNED32|CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t)0x580 + NODE_ID
        },

        // TPDO0 settings
        // 0: The TPDO number, default 0
        // 1: The COB-ID used by TPDO0, provided as a function of the TPDO number
        // 2: How the TPO is triggered, default to manual triggering
        // 3: Inhibit time, defaults to 0
        // 5: Timer trigger time in 1ms units, 0 will disable the timer based triggering
        {
            .Key = CO_KEY(0x1800, 0, CO_UNSIGNED8|CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t)5
        },
        {
            .Key = CO_KEY(0x1800, 1, CO_UNSIGNED32|CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t)0x40000180 + NODE_ID
        },
        {
            .Key = CO_KEY(0x1800, 2, CO_UNSIGNED8|CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t)0xFE
        },
        {
            .Key = CO_KEY(0x1800, 3, CO_UNSIGNED16|CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t)0
        },
        {
            .Key = CO_KEY(0x1800, 5, CO_UNSIGNED16|CO_OBJ_D__R_),
            .Type = CO_TEVENT,
            .Data = (uintptr_t)2000
        },

        // TPDO0 mapping, determins the PDO messages to send when TPDO1 is triggered
        // 0: The number of PDO message associated with the TPDO
        // 1: Link to the first PDO message
        // n: Link to the nth PDO message
        {
            .Key = CO_KEY(0x1A00, 0, CO_UNSIGNED8|CO_OBJ_D__R_),
            .Type = 0,
            .Data = (uintptr_t)1
        },
        {
            .Key = CO_KEY(0x1A00, 1, CO_UNSIGNED32|CO_OBJ_D__R_),
            .Type = 0,
            .Data = CO_LINK(0x2100, 0, 8) // Link to sample data position in dictionary
        },

        // User defined data, this will be where we put elements that can be
        // accessed via SDO and depeneding on configuration PDO
        {
            .Key = CO_KEY(0x2100, 0, CO_UNSIGNED8|CO_OBJ___PRW),
            .Type = 0,
            .Data = (uintptr_t)&bqSettingsStorage.numSettings
        },
        {
            .Key = CO_KEY(0x2101, 0, CO_UNSIGNED32|CO_OBJ___PRW),
            .Type = ((CO_OBJ_TYPE*)&bqSettingsStorage.canOpenInterface),
            .Data = (uintptr_t)&bqSettingsStorage.canOpenInterface
        },
        // End of dictionary marker
        CO_OBJ_DIR_ENDMARK
    };


};


}  // namspace BMS
