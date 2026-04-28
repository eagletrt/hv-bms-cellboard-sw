/*!
 * \file temp.h
 * \date 2024-04-19
 * \author Antonio Gelain [antonio.gelain2@gmail.com]
 * \author Alessandro Giustina [giustinalessandro@gmail.com]
 *
 * \brief Temperature measurment and control
 */

#ifndef TEMP_H
#define TEMP_H

#include <stddef.h>

#include "cellboard-conf.h"
#include "cellboard-def.h"

#include "bms_network.h"

/*! 
 * \defgroup TEMP_LIMIT_C Temperature limits
 *
 * \brief Minimum and maximum allowed cell temperature in celsius 
*/
#define TEMP_MIN_C (-10.f)
#define TEMP_MAX_C (60.f)
/*! \} */

/*!
 * \defgroup TEMP_LIMIT_V Temperature voltage limits
 * \brief Minimum and maximum limit for the temperature voltages in V
 *
 * \details This limit is applied to fit into the polynomial conversion
 * to get a plausible temperature value
 * 
 * \{
 */
#define TEMP_MIN_LIMIT_V (0.f)
#define TEMP_MAX_LIMIT_V (3.f)
/*! \} */

/*!
 * \defgroup TEMP_DISCHARGE_LIMIT_V Discharge resistor voltage limits
 * \brief Minimum and maximum limits for the discharge resistor voltage (V)
 *
 * \details
 * These limits ensure the voltage remains within the valid range
 * for the polynomial conversion, allowing a plausible temperature value.
 * \{
 */
#define TEMP_DISCHARGE_MIN_LIMIT_V (0.0f)
#define TEMP_DISCHARGE_MAX_LIMIT_V (5.1f)
/*! \} */

/*! 
 * \defgroup TEMP_COEFF Polynomial coefficients
 * \brief Coefficients used for the polynomial conversion of the NTC cells temperatures values 
 *
 * \{
 */
#define TEMP_COEFF_0 (148.305319086073000)
#define TEMP_COEFF_1 (-317.553729396941300)
#define TEMP_COEFF_2 (444.564306449468700)
#define TEMP_COEFF_3 (-378.912004657724100)
#define TEMP_COEFF_4 (180.457759604731300)
#define TEMP_COEFF_5 (-44.504609710405890)
#define TEMP_COEFF_6 (4.399756702462762)
/*! \} */

/*!
 * \defgroup TEMP_DISCHARGE_COEFF Polynomial coefficients for discharge resistors
 * \brief Coefficients used for the polynomial conversion of the NTC temperatures values of the discharge resistors 
 * 
 * \{
 */
// #define TEMP_DISCHARGE_COEFF_0 ( 148.305319086073000)
#define TEMP_DISCHARGE_COEFF_0 (178.576844350760100)
#define TEMP_DISCHARGE_COEFF_1 (-191.452565283213000)
#define TEMP_DISCHARGE_COEFF_2 (157.718845424355800)
#define TEMP_DISCHARGE_COEFF_3 (-82.208401759749450)
#define TEMP_DISCHARGE_COEFF_4 (22.346389336008915)
#define TEMP_DISCHARGE_COEFF_5 (-2.510048743779666)
/*! \} */

/*!
 * \brief Type definition for a function callback that sets the muliplexer address
 * 
 * \param address The address to set
 */
typedef void (*temp_set_mux_address_callback)(const uint8_t address);

/*! \brief Type definition for a function callback that starts the ADC conversion */
typedef void (*temp_start_conversion_callback)(void);

/*! \brief Type definition for the array of cells*/
typedef celsius cells_temp[CELLBOARD_SEGMENT_TEMP_SENSOR_COUNT];
/*! \brief Type definition for the array of discharge resistors temperatures */
typedef celsius discharge_temp[CELLBOARD_SEGMENT_DISCHARGE_TEMP_COUNT];

/*!
 * \brief Return code for the temperature module functions
 */
enum TempReturnCode {
    TEMP_OK,           /*!< Function executed successfully */
    TEMP_NULL_POINTER, /*!< A NULL pointer is given as parameter or used inside the function */
    TEMP_BUSY,         /*!< The module is busy and cannot execute the requested function */
    TEMP_OUT_OF_BOUNDS /*!< An index (or pointer) value is greater/lower than the maximum/minimum allowed value */
};

/*!
 * \brief Type definition for the temperature module handler structure
 *
 * \attention Do not use this structure outside of this module
 */
struct TempHandler {
    temp_set_mux_address_callback set_address;       /*!< A pointer to the function callback used to set the multiplexer address */
    temp_start_conversion_callback start_conversion; /*!< A pointer to the function callback used to start the ADC conversion */

    bool busy;                             /*!< Flag that is true if the ADC is busy making conversions */
    uint8_t address;                       /*!< The current address of the multiplexer */
    cells_temp temperatures;               /*!< The cells temperature values in °C */
    discharge_temp discharge_temperatures; /*!< The discharge resistors temperature values in °C */

    bms_cellboard_cells_temperature_converted_t temp_can_payload;               /*!< The canlib payload used to send the cells temperatures data via CAN */
    bms_cellboard_discharge_temperature_converted_t discharge_temp_can_payload; /*!< The canlib payload used to send the discharge resistors temperature data via CAN */
    size_t offset;                                                              /*!< An offset used when the canlib payload is sent to send different values at each transmission */
};

#endif // TEMP_H
