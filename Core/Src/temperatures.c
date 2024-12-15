#include "temperatures.h"
#include "adc.h"
#include "config.h"
//#include <math.h>
#include "structs.h"

extern BmsStatus status;
extern BmsConfig config;
extern ADC_HandleTypeDef hadc;

// Generated lookup table "thermistor_lookup.py"
// TODO: CONFIRM THIS TABLE IS ACCURATE
// Values in deci0ce
static const struct {
    uint16_t adc;
    int16_t temp;
} temp_table[TEMP_TABLE_SIZE] = {
    {0x0E9C, -200}, {0x09CE,  150}, {0x09CE,  150}, {0x09AC,  157},
    {0x098A,  164}, {0x0968,  172}, {0x0946,  179}, {0x0924,  186},
    {0x0902,  194}, {0x08E0,  201}, {0x08BE,  208}, {0x089C,  216},
    {0x087A,  223}, {0x0858,  230}, {0x0836,  238}, {0x0815,  245},
    {0x07F3,  252}, {0x07D2,  260}, {0x07B1,  267}, {0x0790,  274},
    {0x0770,  282}, {0x074F,  289}, {0x072F,  296}, {0x070F,  304},
    {0x06F0,  311}, {0x06D0,  318}, {0x06B1,  326}, {0x0693,  333},
    {0x0674,  340}, {0x0656,  348}, {0x0638,  355}, {0x061B,  362},
    {0x05FE,  370}, {0x05E1,  377}, {0x05C5,  384}, {0x05A9,  392},
    {0x058D,  399}, {0x0572,  406}, {0x0557,  414}, {0x053D,  421},
    {0x0523,  428}, {0x0509,  436}, {0x04F0,  443}, {0x04D7,  450},
    {0x04BE,  458}, {0x04A6,  465}, {0x048E,  472}, {0x0477,  480},
    {0x0460,  487}, {0x044A,  494}, {0x0433,  502}, {0x041E,  509},
    {0x0408,  516}, {0x03F3,  524}, {0x03DF,  531}, {0x03CA,  538},
    {0x03B6,  546}, {0x03A3,  553}, {0x0390,  560}, {0x037D,  568},
    {0x036B,  575}, {0x0359,  582}, {0x0347,  590}, {0x0336,  597},
    {0x0325,  604}, {0x0314,  612}, {0x0304,  619}, {0x02F4,  626},
    {0x02E4,  634}, {0x02D5,  641}, {0x02C6,  648}, {0x02B7,  656},
    {0x02A8,  663}, {0x029A,  670}, {0x028D,  678}, {0x027F,  685},
    {0x0272,  692}, {0x0265,  700}, {0x0265,  700}, {0x010B, 1000},
};

static int16_t convert_adc_to_celsius(uint16_t adc_value) {
    // Handle edge cases beyond the size of the table
    if (adc_value >= temp_table[0].adc) {
        return temp_table[0].temp;
    }
    if (adc_value <= temp_table[TEMP_TABLE_SIZE - 1].adc) {
        return temp_table[TEMP_TABLE_SIZE - 1].temp;
    }

    // Binary search for the closest ADC value
    int left = 0;
    int right = TEMP_TABLE_SIZE - 1;

    while (right - left > 1) {
        int mid = (left + right) / 2;
        if (temp_table[mid].adc == adc_value) {
            return temp_table[mid].temp;
        }

        if (temp_table[mid].adc > adc_value) {
            left = mid;
        } else {
            right = mid;
        }
    }

    // Linear interpolation between points
    uint16_t adc1 = temp_table[left].adc;
    uint16_t adc2 = temp_table[right].adc;
    int16_t temp1 = temp_table[left].temp;
    int16_t temp2 = temp_table[right].temp;

    return temp1 + ((int32_t)(temp2 - temp1) * (adc_value - adc1)) / (adc2 - adc1);
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

//TODO: Reinstate this, not needed really i guess though so...
//HAL_StatusTypeDef TEMP_ReadSTMTemperature() {
//  ADC_ChannelConfTypeDef sConfig = { 0 };
//  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
//  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
//  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
//
//  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
//    return HAL_ERROR;
//  }
//
//  if (HAL_ADC_Start(&hadc) != HAL_OK) {
//    return HAL_ERROR;
//  }
//
//  if (HAL_ADC_PollForConversion(&hadc, 100) != HAL_OK) {
//    return HAL_ERROR;
//  }
//
//  uint32_t adcValue = HAL_ADC_GetValue(&hadc);
//
//  // STM32F0 specific temperature calculation
//  float temperature = ((float) adcValue * VOLTAGE_REFERENCE / ADC_MAX_VALUE - 1.43f) / 0.0043f + 25.0f;
//  status.stmTemperature = (uint16_t) (temperature * 10.0f);
//
//  HAL_ADC_Stop(&hadc);
//
//  return HAL_OK;
//}
