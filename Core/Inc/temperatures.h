#ifndef INC_TEMPERATURES_H_
#define INC_TEMPERATURES_H_

#include <stdint.h>
#include "stm32f0xx_hal.h"

// Constants for NTC calculations
// TODO: get the thermistors from the datasheet
#define NTC_REFERENCE_RESISTANCE 10000.0f
#define NTC_BETA_VALUE 3950.0f
#define NTC_NOMINAL_TEMP 298.15f
#define ADC_MAX_VALUE 4095
#define VOLTAGE_REFERENCE 3.3f

/**
 * @brief Read all configured thermistors and update status
 * @return HAL_OK if read successful (mostly so can return error on fail)
 */
HAL_StatusTypeDef TEMP_ReadAllThermistors();

/**
 * @brief Read and update STM internal temperature
 * @return HAL_OK if read successful
 */
HAL_StatusTypeDef TEMP_ReadSTMTemperature();

#endif /* INC_TEMPERATURES_H_ */
