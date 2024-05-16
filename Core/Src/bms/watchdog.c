/**
 * @file watchdog.c
 * @date 2024-04-29
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions used to constantly check for the presence of some signals that should
 * always be present
 */

#include "watchdog.h"

#include "bms_network.h"
#include "bms_watchdog.h"

// TODO: Use custom watchdogs or merge?
#ifdef DO_NOT_DEFINE

#define WATCHDOG_INDEX_COUNT 1

bms_watchdog watchdog;

static can_index watched_ids[WATCHDOG_INDEX_COUNT] = {
    BMS_MAINBOARD_STATUS_INDEX
};

WatchdogReturnCode watchdog_init(void) {
    for (size_t i = 0; i < WATCHDOG_INDEX_COUNT; ++i) {
        // Deactivate the watchdog
        CANLIB_BITCLEAR_ARRAY(watcdog.activated, watched_ids[i]);
    }
    return WATCHDOG_OK;
}

inline bool watchdog_is_active(can_index index) {
    return index >= bms_MESSAGE_COUNT ? false : CANLIB_BITTEST_ARRAY(watchdog.activated, index);
}

inline bool watchdog_is_timed_out(can_index index) {
    return index >= bms_MESSAGE_COUNT ? false : CANLIB_BITTEST_ARRAY(watchdog.timeout, index);
}

WatchdogReturnCode watchdog_start(can_index index) {
    if (index >= bms_MESSAGE_COUNT)
        return WATCHDOG_INVALID_ID;

    // Activate the watchdog
    CANLIB_BITSET_ARRAY(wathdog.activated, index);
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_start_all(void) {
    for (size_t i = 0; i < WATCHDOG_INDEX_COUNT; ++i) {
        // Activate the watchdog
        CANLIB_BITSET_ARRAY(wathdog.activated, watched_ids[i]);
    }
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_stop(can_index index) {
    if (index >= bms_MESSAGE_COUNT)
        return WATCHDOG_INVALID_ID;

    // Deactivate the watchdog
    CANLIB_BITCLEAR_ARRAY(wathdog.activated, index);
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_stop_all(void) {
    for (size_t i = 0; i < WATCHDOG_INDEX_COUNT; ++i) {
        // Activate the watchdog
        CANLIB_BITCLEAR_ARRAY(wathdog.activated, watched_ids[i]);
    }
    return WATCHDOG_OK;
}

WatchdogReturnCode watchdog_reset(can_index index, time timestamp) {
    if (index >= bms_MESSAGE_COUNT)
        return WATCHDOG_INVALID_ID;
    can_id id = bms_id_from_index(index);
    // TODO: Change watchdog reset to get index instead of id
    bms_watchdog_reset(&watchdog, id, timestamp);
    return WATCHDOG_OK;
}

// TODO: Do things when watchdog has timed out
WatchdogReturnCode watchdog_routine(time timestamp) {
    // Check if the watchdog has timed out
    bms_watchdog_timeout(&watchdog, timestamp);

    WatchdogReturnCode ret = WATCHDOG_OK;
    for (size_t i = 0; i < WATCHDOG_INDEX_COUNT; ++i) {
        if (CANLIB_BITTEST_ARRAY(watchdog.activated, watched_ids[i]) &&
            CANLIB_BITTEST_ARRAY(watchdog.timeout, watched_ids[i]))
        {
            // Stop the timed out watchdog
            CANLIB_BITCLEAR_ARRAY(wathdog.activated, watched_ids[i]);
            // TODO: Return immediately on timeout?
            ret = WATCHDOG_TIMEOUT;
        }
    }
    return ret;
}

#ifdef CONF_WATCHDOG_STRINGS_ENABLE

static char * watchdog_module_name = "watchdog"

static char * watchdog_return_code_name[] = {
    [WATCHDOG_OK] = "ok",
    [WATCHDOG_INVALID_ID] = "invalid id",
    [WATCHDOG_TIMEOUT] = "timeout"
};

static char * watchdog_return_code_description[] = {
    [WATCHDOG_OK] = "executed succesfully",
    [WATCHDOG_INVALID_ID] = "an invalid CAN identifier was used",
    [WATCHDOG_TIMEOUT] = "a watchdog has timed out"
};

#endif // CONF_WATCHDOG_STRINGS_ENABLE

#endif // CONF_WATCHDOG_MODULE_ENABLE
