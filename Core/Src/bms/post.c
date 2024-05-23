/**
 * @file post.c
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Power-On Self Test function to check that every internal system and
 * peripheral is working correctly
 */

#include "post.h"

#include "identity.h"
#include "programmer.h"
#include "timebase.h"
#include "volt.h"
#include "temp.h"
#include "bal.h"

#ifdef CONF_POST_MODULE_ENABLE

/* !!! ATTENTION: the order in which the init functions are called matters !!! */
PostReturnCode post_run(PostInitData data) {
    if (data.id >= CELLBOARD_ID_COUNT)
        return POST_INVALID_CELLBOARD_ID;
    if (data.system_reset == NULL ||
        data.can_send == NULL ||
        data.spi_send == NULL ||
        data.spi_send_receive == NULL ||
        data.led_set == NULL ||
        data.led_toggle == NULL)
        return POST_NULL_POINTER;

    identity_init(data.id); // Have to be always called first

    // TODO: Test that every peripheral is working
    // TODO: Call all the init functions
    // TODO: Return errors based on function return codes
    
    // Some return values are ignored because always OK
    (void)timebase_init(1U);
    (void)bms_manager_init(data.spi_send, data.spi_send_receive); // NULL check for callbacks above
    (void)volt_init();
    (void)temp_init();
    (void)can_comm_init(data.can_send); // NULL check for send above
    (void)bal_init();
    (void)programmer_init(data.system_reset);
    (void)led_init(data.led_set, data.led_toggle); // Id and NULL checks above
 
    return POST_OK;
}

#ifdef CONF_POST_STRINGS_ENABLE

static char * post_module_name = "post";

static char * post_return_code_name[] = {
    [POST_OK] = "ok",
    [POST_INVALID_CELLBOARD_ID] = "invalid cellboard id",
    [POST_NULL_POINTER] = "null pointer",
    [POST_WATCHDOG_INVALID_MESSAGE] = "invalid watchdog message"
};

static char * post_return_code_description[] = {
    [POST_OK] = "executed successfully",
    [POST_INVALID_CELLBOARD_ID] = "the given id does not correspond to any valid cellboard identifier",
    [POST_NULL_POINTER] = "attempt to dereference a null pointer",
    [POST_WATCHDOG_INVALID_MESSAGE] = "the watchdogs are using a non valid can message"
};

#endif // CONF_POST_STRINGS_ENABLE

#endif // CONF_POST_MODULE_ENABLE
