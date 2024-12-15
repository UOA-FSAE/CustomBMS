#ifndef INC_TEMPERATURES_H_
#define INC_TEMPERATURES_H_

#include <stdint.h>
#include "stm32f0xx_hal.h"

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
