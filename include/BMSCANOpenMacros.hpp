#pragma once

#include <EVT/io/CANOpenMacros.hpp>

/**
 * This macro creates a TPDO settings object for an extra node on a single
 * device. This macro itself is abstract, allowing it to be used with any TPDO
 * number supported by CANOpen.
 *
 * @param TPDO_NUMBER (integer) the TPDO number this settings object is for.
 * @param TRANSMISSION_TYPE (hex) the type of transmission to make. You should use TRANSMIT_PDO_TRIGGER_TIMER.
 * @param INHIBIT_TIME (integer) The amount of time (in 100μs increments) that must pass before another TPDO message can be sent.
 * @param INTERVAL (integer) the time trigger (in ms) that the TPDO sends on (0 = disable).
 *
*/
#define EXTRA_TRANSMIT_PDO_SETTINGS_OBJECT_18XX(TPDO_NUMBER, TRANSMISSION_TYPE, INHIBIT_TIME, INTERVAL) \
    {                                                                                                   \
        /* TPDO #N Settings Object */                                                                   \
        .Key = CO_KEY(0x1800 + TPDO_NUMBER, 0x00, CO_OBJ_D___R_),                                       \
        .Type = CO_TUNSIGNED8,                                                                          \
        .Data = (CO_DATA) 0x05,                                                                         \
    },                                                                                                  \
        {                                                                                               \
            /* COB-ID used by TPDO  180h+TPDO Node-ID*/                                                 \
            .Key = CO_KEY(0x1800 + TPDO_NUMBER, 0x01, CO_OBJ_DN__R_),                                   \
            .Type = CO_TPDO_ID,                                                                         \
            .Data = (CO_DATA) CO_COBID_TPDO_DEFAULT(TPDO_NUMBER - 4) + 0x10,                            \
        },                                                                                              \
        {                                                                                               \
            /* Transmission type */                                                                     \
            .Key = CO_KEY(0x1800 + TPDO_NUMBER, 0x02, CO_OBJ_D___R_),                                   \
            .Type = CO_TPDO_TYPE,                                                                       \
            .Data = (CO_DATA) TRANSMISSION_TYPE,                                                        \
        },                                                                                              \
        {                                                                                               \
            /* Inhibit time with LSB 100us (0=disable) */                                               \
            .Key = CO_KEY(0x1800 + TPDO_NUMBER, 0x03, CO_OBJ_D___R_),                                   \
            .Type = CO_TUNSIGNED16,                                                                     \
            .Data = (CO_DATA) INHIBIT_TIME,                                                             \
        },                                                                                              \
    { /* Event timer LSB 1ms (0=disable) */                                                             \
        .Key = CO_KEY(0x1800 + TPDO_NUMBER, 0x05, CO_OBJ_D___R_),                                       \
        .Type = CO_TPDO_EVENT,                                                                          \
        .Data = (CO_DATA) INTERVAL,                                                                     \
    }
