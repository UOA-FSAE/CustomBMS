#include "temperatures.h"
#include "adc.h"
#include <math.h>
#include "structs.h"

extern BmsStatus status;
extern BmsConfig config;
extern ADC_HandleTypeDef hadc;

static float convert_adc_to_celsius(uint16_t adcValue) {
  // NGL, this function was made in ChatGPT, I assume it's correct but it needs testing
  float voltage = (float) adcValue * VOLTAGE_REFERENCE / ADC_MAX_VALUE;
  float resistance = NTC_REFERENCE_RESISTANCE * ((VOLTAGE_REFERENCE / voltage) - 1.0f);
  float steinhart = logf(resistance / NTC_REFERENCE_RESISTANCE) / NTC_BETA_VALUE;
  steinhart += 1.0f / NTC_NOMINAL_TEMP;
  return ((1.0f / steinhart) - 273.15f) * 10.0f; // Return in deci-Celsius
}

HAL_StatusTypeDef TEMP_ReadAllThermistors(void) {
  status.maxTemp = 0;
  status.minTemp = UINT16_MAX;

  // Configure ADC for scanning NTC channels
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

  for(uint8_t i = 0; i < config.numberOfThermistors; i++) {
      // Configure channel
      sConfig.Channel = ADC_CHANNEL_0 + i;
      if(HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
          return HAL_ERROR;
      }

      // Start conversion for this channel
      if(HAL_ADC_Start(&hadc) != HAL_OK) {
          return HAL_ERROR;
      }

      // Wait for conversion
      //TODO: confirm if this can be used in interrupt or if it shits the bed
      if(HAL_ADC_PollForConversion(&hadc, 100) != HAL_OK) {
          HAL_ADC_Stop(&hadc);
          return HAL_ERROR;
      }

      // Read and convert value
      uint16_t adcValue = HAL_ADC_GetValue(&hadc);
      uint16_t temp = (uint16_t)convert_adc_to_celsius(adcValue);

      status.cellTemperatures[i] = temp;

      if(temp < status.minTemp) {
          status.minTemp = temp;
      }
      if(temp > status.maxTemp) {
          status.maxTemp = temp;
      }

      HAL_ADC_Stop(&hadc);
  }

  // Update warning flag
    status.cellWarningTemperature = (status.maxTemp >= (config.warningTemperature * 10)) ? 1 : 0;

    // Update shutdown flag
    status.cellShutdownTemperature = (status.maxTemp >= (config.shutdownTemperature * 10)) ? 1 : 0;

  return HAL_OK;
}

HAL_StatusTypeDef TEMP_ReadSTMTemperature() {
  ADC_ChannelConfTypeDef sConfig = { 0 };
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
    return HAL_ERROR;
  }

  if (HAL_ADC_Start(&hadc) != HAL_OK) {
    return HAL_ERROR;
  }

  if (HAL_ADC_PollForConversion(&hadc, 100) != HAL_OK) {
    return HAL_ERROR;
  }

  uint32_t adcValue = HAL_ADC_GetValue(&hadc);

  // STM32F0 specific temperature calculation
  float temperature = ((float) adcValue * VOLTAGE_REFERENCE / ADC_MAX_VALUE - 1.43f) / 0.0043f + 25.0f;
  status.stmTemperature = (uint16_t) (temperature * 10.0f);

  HAL_ADC_Stop(&hadc);

  return HAL_OK;
}
