/*!
 * \file post-api.c
 * \date 2024-04-16
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Power-On Self Test function to check that every internal system and
 * peripheral is working correctly
 */

#include "post-api.h"

#include "error.h"
#include "identity-api.h"
#include "timebase.h"
#include "bal-api.h"
#include "temp-api.h"
#include "programmer-api.h"
#include "led-api.h"
#include "volt-api.h"

#ifdef CONF_POST_MODULE_ENABLE

/*!
 * \brief Initialize all the cellboard modules
 *
 * \attention The order in which the init functions are called matters
 *
 * \param data A pointer to the initialization data
 *
 * \retval POST_RC_OK
 */
enum PostReturnCode prv_post_api_modules_init(const struct PostInitData *const data) {
    /*
     * The error and identity initialization functions have to be executed
     * before every other function to ensure the proper functionality
     */
    if (error_init(data->system_reset) != ERROR_OK) {
        return POST_RC_UNINITIALIZED;
    }
    identity_api_init(data->id);

    /**
     * Some of the function return values can be ignored because they are either
     * always OK or some assertion can be made (like for the NULL checks)
     */
    (void)timebase_init(1U);
    (void)bms_manager_api_init(data->spi_send, data->spi_send_receive);
    (void)volt_api_init();
    (void)temp_api_init(data->gpio_set_address, data->adc_start);
    (void)can_comm_init(data->can_send);
    (void)bal_api_init();
    (void)programmer_api_init(data->system_reset);
    (void)led_api_init(data->led_set, data->led_toggle);

    return POST_RC_OK;
}

enum PostReturnCode prv_post_api_module_setup(void) {
    timebase_set_enable(true);
    can_comm_enable_all();
    led_api_set_enable(true);
    return POST_RC_OK;
}

enum PostReturnCode post_api_run(const struct PostInitData data) {
    if (data.id >= CELLBOARD_ID_COUNT) {
        return POST_RC_INVALID_CELLBOARD_ID;
    }
    if (data.system_reset == NULL ||
        data.can_send == NULL ||
        data.spi_send == NULL ||
        data.spi_send_receive == NULL ||
        data.led_set == NULL ||
        data.led_toggle == NULL) {
        return POST_RC_NULL_POINTER;
    }
    enum PostReturnCode post_code = prv_post_api_modules_init(&data);
    if (post_code != POST_RC_OK) {
        return post_code;
    }

    post_code = prv_post_api_module_setup();

    // TODO: Test that every peripheral is working?

    return post_code;
}

#ifdef CONF_POST_STRINGS_ENABLE

EAGLETRT_STATIC char *post_module_name = "post";

EAGLETRT_STATIC char *post_return_code_name[] = {
    [POST_RC_OK] = "ok",
    [POST_RC_UNINITIALIZED] = "uninitialized",
    [POST_RC_INVALID_CELLBOARD_ID] = "invalid cellboard id",
    [POST_RC_NULL_POINTER] = "null pointer"
};

EAGLETRT_STATIC char *post_return_code_description[] = {
    [POST_RC_OK] = "executed successfully",
    [POST_RC_UNINITIALIZED] = "a module has not been initialized correctly",
    [POST_RC_INVALID_CELLBOARD_ID] = "the given id does not correspond to any valid cellboard identifier",
    [POST_RC_NULL_POINTER] = "attempt to dereference a null pointer"
};

#endif // CONF_POST_STRINGS_ENABLE

#endif // CONF_POST_MODULE_ENABLE
