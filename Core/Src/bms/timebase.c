/**
 * @file timebase.c
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Functions to manage periodic tasks at certain intervals
 */

#include "timebase.h"

#include "bms_network.h"
#include "identity.h"
#include "fsm.h"
#include "volt.h"
#include "temp.h"

#ifdef CONF_TIMEBASE_MODULE_ENABLE

/**
 * @brief Check if the given interval has elapsed
 *
 * @param INTERVAL_ID The interval identifier (see 'TimebaseIntervalId' struct)
 */
#define _TIMEBASE_INTERVAL_CHECK_ELAPSED(INTERVAL_ID) ((htimebase.t % interval_ms[INTERVAL_ID]) == 0U)

/**
 * @brief Check if the previous interval has not been handled yet
 *
 * @details This macro checks if the interval flag of the given interval is set to 1
 * That means that the tasks that should have been executed in the previous interval
 * have not been completed yet
 *
 * @param INTERVAL_ID The interval identifier (see 'TimebaseIntervalId' struct)
 */
#define _TIMEBASE_INTERVAL_CHECK_OVERRUN(INTERVAL_ID) ((htimebase.flags & interval_flag[INTERVAL_ID]) != 0U)

/**
 * @brief Get, set or reset the flag of a given interval
 *
 * @details The get flag macro return the value of the bit flag and not the flag itself
 *
 * @param INTERVAL_ID The interval identifier (see 'TimebaseIntervalId' struct)
 */
#define _TIMEBASE_INTERVAL_GET_FLAG(INTERVAL_ID) ((htimebase.flags &= interval_flag[INTERVAL_ID]) != 0U)
#define _TIMEBASE_INTERVAL_RESET_FLAG(INTERVAL_ID) (htimebase.flags &= ~interval_flag[INTERVAL_ID])
#define _TIMEBASE_INTERVAL_SET_FLAG(INTERVAL_ID) (htimebase.flags |= interval_flag[INTERVAL_ID])

/**
 * @brief Check if a given interval is elapsed and, in that case, set the appropriate flag
 *
 * @param INTERVAL_ID The interval identifier (see 'TimebaseIntervalId' struct)
 * @param OUT A variable of type 'TimebaseReturnCode' that can be modified
 */
#define _TIMEBASE_INTERVAL_CHECK(INTERVAL_ID, OUT) \
( \
    _TIMEBASE_INTERVAL_CHECK_ELAPSED(INTERVAL_ID) && \
    (!_TIMEBASE_INTERVAL_CHECK_OVERRUN(INTERVAL_ID) || (OUT = TIMEBASE_BUSY) || 1U) && \
    _TIMEBASE_INTERVAL_SET_FLAG(INTERVAL_ID) \
)

/**
 * @brief Index of the intervals information
 */
typedef enum {
    TIMEBASE_INTERVAL_ID_10_T = 0U,
    TIMEBASE_INTERVAL_ID_20_T,
    TIMEBASE_INTERVAL_ID_50_T,
    TIMEBASE_INTERVAL_ID_100_T,
    TIMEBASE_INTERVAL_ID_200_T,
    TIMEBASE_INTERVAL_ID_500_T,
    TIMEBASE_INTERVAL_ID_1000_T
} TimebaseIntervalId;

/**
 * @brief Position of the bit flag
 */
const uint16_t interval_flag_pos[] = {
    [TIMEBASE_INTERVAL_ID_10_T] = 1U,
    [TIMEBASE_INTERVAL_ID_20_T] = 2U,
    [TIMEBASE_INTERVAL_ID_50_T] = 4U,
    [TIMEBASE_INTERVAL_ID_100_T] = 8U,
    [TIMEBASE_INTERVAL_ID_200_T] = 9U,
    [TIMEBASE_INTERVAL_ID_500_T] = 11U,
    [TIMEBASE_INTERVAL_ID_1000_T] = 14U
};

/**
 * @brief Bit of the flag interval
 */
const uint16_t interval_flag[] = {
    [TIMEBASE_INTERVAL_ID_10_T] = 1U << interval_flag_pos[TIMEBASE_INTERVAL_ID_10_T],
    [TIMEBASE_INTERVAL_ID_20_T] = 1U << interval_flag_pos[TIMEBASE_INTERVAL_ID_20_T],
    [TIMEBASE_INTERVAL_ID_50_T] = 1U << interval_flag_pos[TIMEBASE_INTERVAL_ID_50_T],
    [TIMEBASE_INTERVAL_ID_100_T] = 1U << interval_flag_pos[TIMEBASE_INTERVAL_ID_100_T],
    [TIMEBASE_INTERVAL_ID_200_T] = 1U << interval_flag_pos[TIMEBASE_INTERVAL_ID_200_T],
    [TIMEBASE_INTERVAL_ID_500_T] = 1U << interval_flag_pos[TIMEBASE_INTERVAL_ID_500_T],
    [TIMEBASE_INTERVAL_ID_1000_T] = 1U << interval_flag_pos[TIMEBASE_INTERVAL_ID_1000_T]
};

/**
 * @brief Task intervals in ms
 */
const ticks interval_ms[] = {
    [TIMEBASE_INTERVAL_ID_10_T] = 10U,
    [TIMEBASE_INTERVAL_ID_20_T] = 20U,
    [TIMEBASE_INTERVAL_ID_50_T] = 50U,
    [TIMEBASE_INTERVAL_ID_100_T] = 100U,
    [TIMEBASE_INTERVAL_ID_200_T] = 200U,
    [TIMEBASE_INTERVAL_ID_500_T] = 500U,
    [TIMEBASE_INTERVAL_ID_1000_T] = 1000U,
};

/**
 * @brief Timebase handler structure
 *
 * @param enabled True if the timebase is running, false otherwise
 * @param resolution Number of ms that represent one tick
 * @param t The current number of ticks
 * @param flags A 16 bit flag variable used to
 */
static struct {
    bool enabled;
    time resolution; // in ms
    ticks t;
    bit_flag16 flags;
} htimebase;

TimebaseReturnCode timebase_init(time resolution_ms) {
    htimebase.enabled = false;
    htimebase.resolution = resolution_ms <= 0U ? 1U : resolution_ms;
    htimebase.t = 0U;
    return TIMEBASE_OK;
}

void timebase_set_enable(bool enabled) {
    htimebase.enabled = enabled;
}

TimebaseReturnCode timebase_inc_tick(void) {
    if (!htimebase.enabled)
        return TIMEBASE_DISABLED;

    ++htimebase.t;
    TimebaseReturnCode ret = TIMEBASE_OK;

    /* !!! The value of ret could be modified by the macros !!! */
    _TIMEBASE_INTERVAL_CHECK(TIMEBASE_INTERVAL_ID_10_T, ret);
    _TIMEBASE_INTERVAL_CHECK(TIMEBASE_INTERVAL_ID_20_T, ret);
    _TIMEBASE_INTERVAL_CHECK(TIMEBASE_INTERVAL_ID_50_T, ret);
    _TIMEBASE_INTERVAL_CHECK(TIMEBASE_INTERVAL_ID_100_T, ret);
    _TIMEBASE_INTERVAL_CHECK(TIMEBASE_INTERVAL_ID_200_T, ret);
    _TIMEBASE_INTERVAL_CHECK(TIMEBASE_INTERVAL_ID_500_T, ret);
    _TIMEBASE_INTERVAL_CHECK(TIMEBASE_INTERVAL_ID_1000_T, ret);
    return ret;
}

ticks timebase_get_tick(void) {
    return htimebase.t;
}

time timebase_get_time(void) {
    return htimebase.t * htimebase.resolution;
}

TimebaseReturnCode timebase_routine(void) {
    if (!htimebase.enabled)
        return TIMEBASE_DISABLED;

    size_t byte_size = 0U;
    void * payload = NULL;

    // TODO: Add tasks at given intervals
    if (_TIMEBASE_INTERVAL_GET_FLAG(TIMEBASE_INTERVAL_ID_10_T)) {

    }
    else if (_TIMEBASE_INTERVAL_GET_FLAG(TIMEBASE_INTERVAL_ID_20_T)) {
        // FSM status
        payload = fsm_get_can_payload(&byte_size);
        can_comm_tx_add(BMS_CELLBOARD_STATUS_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
    }
    else if (_TIMEBASE_INTERVAL_GET_FLAG(TIMEBASE_INTERVAL_ID_50_T)) {
        // Voltages and temperatures
        payload = volt_get_canlib_payload(&byte_size);
        can_comm_tx_add(BMS_CELLS_VOLTAGES_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
        payload = temp_get_canlib_payload(&byte_size);
        can_comm_tx_add(BMS_CELLS_TEMPERATURES_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);
    }
    else if (_TIMEBASE_INTERVAL_GET_FLAG(TIMEBASE_INTERVAL_ID_100_T)) {

    }
    else if (_TIMEBASE_INTERVAL_GET_FLAG(TIMEBASE_INTERVAL_ID_200_T)) {

    }
    else if (_TIMEBASE_INTERVAL_GET_FLAG(TIMEBASE_INTERVAL_ID_500_T)) {

    }
    else if (_TIMEBASE_INTERVAL_GET_FLAG(TIMEBASE_INTERVAL_ID_1000_T)) {
        // Version
        payload = identity_get_can_payload(&byte_size);
        can_comm_tx_add(BMS_CELLBOARD_VERSION_INDEX, CAN_FRAME_TYPE_DATA, payload, byte_size);

        // Check watchdog
        watchdog_routine(timebase_get_time());
    }
    return TIMEBASE_OK;
}

#ifdef CONF_TIMEBASE_STRINGS_ENABLE

static char * timebase_module_name = "timebase";

static char * timebase_return_code_name[] = {
    [TIMEBASE_OK] = "ok",
    [TIMEBASE_DISABLED] = "disabled",
    [TIMEBASE_BUSY] = "busy"
};

static char * timebase_return_code_description[] = {
    [TIMEBASE_OK] = "executed successfully",
    [TIMEBASE_DISABLE] = "the timebase is not enabled",
    [TIMEBASE_BUSY] = "the timebase couldn't perform the requested operation"
};

#endif // CONF_TIMEBASE_STRINGS_ENABLE

#endif // CONF_TIMEBASE_MODULE_ENABLE_H
