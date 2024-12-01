/**
 * @file post.c
 * @date 2024-04-16
 * @author Antonio Gelain [antonio.gelain2@gmail.com]
 *
 * @brief Power-On Self Test function to check that every internal system and
 * peripheral is working correctly
 */

#include "post.h"

#include "error.h"
#include "identity.h"
#include "programmer.h"
#include "timebase.h"
#include "volt.h"
#include "temp.h"
#include "bal.h"

#ifdef CONF_POST_MODULE_ENABLE

/**
 * @brief Initialize all the cellboard modules
 * 
 * @attention The order in which the init functions are called matters
 *
 * @param data A pointer to the initialization data
 *
 * @return PostReturnCode
 *     - POST_OK
 */
PostReturnCode _post_modules_init(const PostInitData * const data) {
    /*
     * The error and identity initialization functions have to be executed
     * before every other function to ensure the proper functionality
     */
    if (error_init(data->system_reset) != ERROR_OK)
        return POST_UNINITIALIZED;
    identity_init(data->id);

    /**
     * Some of the function return values can be ignored because they are either
     * always OK or some assertion can be made (like for the NULL checks)
     */
    (void)timebase_init(1U);
    (void)bms_manager_init(data->spi_send, data->spi_send_receive);
    (void)volt_init();
    (void)temp_init(data->gpio_set_address, data->adc_start);
    (void)can_comm_init(data->can_send);
    (void)bal_init();
    (void)programmer_init(data->system_reset);
    (void)led_init(data->led_set, data->led_toggle);

    return POST_OK;
}

PostReturnCode _post_module_setup(void) {
    timebase_set_enable(true);
    can_comm_enable_all();
    led_set_enable(true);
    return POST_OK;
}

PostReturnCode post_run(const PostInitData data) {
    if (data.id >= CELLBOARD_ID_COUNT)
        return POST_INVALID_CELLBOARD_ID;
    if (data.system_reset == NULL ||
        data.can_send == NULL ||
        data.spi_send == NULL ||
        data.spi_send_receive == NULL ||
        data.led_set == NULL ||
        data.led_toggle == NULL)
        return POST_NULL_POINTER;

    // Module initialization
    PostReturnCode post_code = _post_modules_init(&data);
    if (post_code != POST_OK)
        return post_code;

    // Module confiuration
    post_code = _post_module_setup();

    // TODO: Test that every peripheral is working

    return post_code;
}

#ifdef CONF_POST_STRINGS_ENABLE

_STATIC char * post_module_name = "post";

_STATIC char * post_return_code_name[] = {
    [POST_OK] = "ok",
    [POST_UNINITIALIZED] = "uninitialized",
    [POST_INVALID_CELLBOARD_ID] = "invalid cellboard id",
    [POST_NULL_POINTER] = "null pointer"
};

_STATIC char * post_return_code_description[] = {
    [POST_OK] = "executed successfully",
    [POST_UNINITIALIZED] = "a module has not been initialized correctly",
    [POST_INVALID_CELLBOARD_ID] = "the given id does not correspond to any valid cellboard identifier",
    [POST_NULL_POINTER] = "attempt to dereference a null pointer"
};

#endif // CONF_POST_STRINGS_ENABLE

#endif // CONF_POST_MODULE_ENABLE
