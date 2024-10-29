#include "main.h"
#include "config.h"
#include "BMSCommands.h"
#include "structs.h"
#include "stdint.h"
#include "stdbool.h"

extern SPI_HandleTypeDef hspi1;

extern BmsStatus status;
extern BmsConfig config;


/* Helper Functions*/
uint16_t BMS_CalculatePEC(uint8_t *dataArray, uint8_t arrayLength) {
	// Initialize the PEC to 000000000010000 (0x0010)
	uint16_t pec = 0x0010;

	// Process each byte of the data array
	uint8_t in0, in3, in4, in7, in8, in10, in14;
	for (uint8_t byteIndex = 0; byteIndex < arrayLength; byteIndex++) {

		// Process each bit of the current byte
		for (int bitIndex = 7; bitIndex >= 0; bitIndex--) {
			// Get the incoming bit (DIN)
			uint8_t din = (dataArray[byteIndex] >> bitIndex) & 0x01;

			// Calculate intermediate values
			in0 = din ^ ((pec >> 14) & 0x01);
			in3 = in0 ^ ((pec >> 2) & 0x01);
			in4 = in0 ^ ((pec >> 3) & 0x01);
			in7 = in0 ^ ((pec >> 6) & 0x01);
			in8 = in0 ^ ((pec >> 7) & 0x01);
			in10 = in0 ^ ((pec >> 9) & 0x01);
			in14 = in0 ^ ((pec >> 13) & 0x01);

			// Update the PEC register according to the specified bit positions
			uint16_t newPEC = 0;
			newPEC |= (in14 << 14);
			newPEC |= ((pec >> 12) & 0x01) << 13;
			newPEC |= ((pec >> 11) & 0x01) << 12;
			newPEC |= ((pec >> 10) & 0x01) << 11;
			newPEC |= (in10 << 10);
			newPEC |= ((pec >> 8) & 0x01) << 9;
			newPEC |= (in8 << 8);
			newPEC |= (in7 << 7);
			newPEC |= ((pec >> 5) & 0x01) << 6;
			newPEC |= ((pec >> 4) & 0x01) << 5;
			newPEC |= (in4 << 4);
			newPEC |= (in3 << 3);
			newPEC |= ((pec >> 1) & 0x01) << 2;
			newPEC |= ((pec >> 0) & 0x01) << 1;
			newPEC |= in0;

			pec = newPEC;
		}
	}

	return pec << 1;
}

void BMS_SendCommand(uint16_t command, bool toggleCS) {
	uint8_t cmd[4];

	cmd[0] = (command >> 8) & 0xFF;
	cmd[1] = command & 0xFF;

	// Calculate PEC (Packet Error Code) for the command
	uint16_t pec = BMS_CalculatePEC(cmd, 2);
	cmd[2] = (pec >> 8) & 0xFF;
	cmd[3] = pec & 0xFF;

	// Pull CS (Chip Select) low to start communication
	if (toggleCS) {
		HAL_GPIO_WritePin(BMS_CS_GPIO_Port, BMS_CS_Pin, GPIO_PIN_RESET);
	}

	if (HAL_SPI_Transmit(&hspi1, cmd, sizeof(cmd), HAL_MAX_DELAY) != HAL_OK) {
		// If transmission fails, pull CS high and return the status
		HAL_GPIO_WritePin(BMS_CS_GPIO_Port, BMS_CS_Pin, GPIO_PIN_SET);
		Error_Handler();
	}

	// Pull CS (Chip Select) high to end communication
	if (toggleCS) {
		HAL_GPIO_WritePin(BMS_CS_GPIO_Port, BMS_CS_Pin, GPIO_PIN_SET);
	}

}

void BMS_WriteRegister(uint16_t command, uint8_t *data, uint8_t dataSize) {
	// Pull CS (Chip Select) low to start communication
	HAL_GPIO_WritePin(BMS_CS_GPIO_Port, BMS_CS_Pin, GPIO_PIN_RESET);

	// Send the command to write the register
	BMS_SendCommand(command, false);

	// Calculate the PEC for the data
	uint16_t pec = BMS_CalculatePEC(data, dataSize);

	// Create a new array that includes the data and the PEC
	uint8_t dataWithPEC[dataSize + 2];

	for (uint8_t i = 0; i < dataSize; i++) {
		dataWithPEC[i] = data[i];
	}

	dataWithPEC[dataSize] = (pec >> 8) & 0xFF; // PEC high byte
	dataWithPEC[dataSize + 1] = pec & 0xFF;    // PEC low byte

	// Transmit the data with PEC
	if (HAL_SPI_Transmit(&hspi1, dataWithPEC, dataSize + 2, HAL_MAX_DELAY)
			!= HAL_OK) {
		// If transmission fails, pull CS high and return the status
		HAL_GPIO_WritePin(BMS_CS_GPIO_Port, BMS_CS_Pin, GPIO_PIN_SET);
		Error_Handler();
	}

	// Pull CS (Chip Select) high to end communication
	HAL_GPIO_WritePin(BMS_CS_GPIO_Port, BMS_CS_Pin, GPIO_PIN_SET);
}

void BMS_ReadRegister(uint16_t command, uint8_t *data, uint8_t dataSize) {

	// Pull CS (Chip Select) low to start communication
	HAL_GPIO_WritePin(BMS_CS_GPIO_Port, BMS_CS_Pin, GPIO_PIN_RESET);

	// Send the command to read the register
	BMS_SendCommand(command, false);

	// Receive the response
	if (HAL_SPI_Receive(&hspi1, data, dataSize, HAL_MAX_DELAY) != HAL_OK) {
		// If transmission fails, pull CS high and return the status
		HAL_GPIO_WritePin(BMS_CS_GPIO_Port, BMS_CS_Pin, GPIO_PIN_SET);
		Error_Handler();
	}
	// Pull CS (Chip Select) high to end communication
	HAL_GPIO_WritePin(BMS_CS_GPIO_Port, BMS_CS_Pin, GPIO_PIN_SET);

}

void BMS_SetConfigurationRegisters(uint8_t *configA, uint8_t *configB) {
	// Write the new configuration back
	BMS_WriteRegister(WRITE_CONFIG_REGISTER_A, configA, 6);
	BMS_WriteRegister(WRITE_CONFIG_REGISTER_B, configB, 6);
}

void BMS_GetConfigurationRegisters(uint8_t *configA, uint8_t *configB) {
	// Read the current configuration
	BMS_ReadRegister(READ_CONFIG_REGISTER_A, configA, 6);
	BMS_ReadRegister(READ_CONFIG_REGISTER_B, configB, 6);
}

/* Configuration functions */
void BMS_InitConfigRegisters() {
	// hard codes the registers, wiping everything
	uint8_t configA[6] = { 0 };
	uint8_t configB[6] = { 0 };

	configA[0] = 0b00000100; // Enable reference, ADCOPT 0

	// Set the overvoltage limit (12-bit value)
	uint16_t overVoltageLimit = (config.overVoltageMV * 10) / 16;
	configA[2] = (configA[2] & 0x0F) | ((overVoltageLimit & 0x0F) << 4);
	configA[3] = (overVoltageLimit >> 4) & 0xFF;

	// Set the overvoltage limit (12-bit value)
	uint16_t underVoltageLimit = (config.underVoltageMV * 10) / 16;
	configA[2] = (configA[2] & 0x0F) | ((underVoltageLimit & 0x0F) << 4);
	configA[3] = (underVoltageLimit >> 4) & 0xFF;

	// Initialize config registers with default values
	BMS_SetConfigurationRegisters(configA, configB);
}

void BMS_SetOverVoltageLimit(uint16_t voltageInMv) {
	uint8_t configA[6] = { 0 };
	uint8_t configB[2] = { 0 };

	// Get the up-to-date configuration registers
	BMS_GetConfigurationRegisters(configA, configB);
	// Convert voltage from mV to the 12-bit overvoltage limit value
	uint16_t voltageLimit = (voltageInMv * 10) / 16;

	// Set the overvoltage limit (12-bit value)
	configA[2] = (configA[2] & 0x0F) | ((voltageLimit & 0x0F) << 4);
	configA[3] = (voltageLimit >> 4) & 0xFF;

	// Write the new configuration back
	BMS_SetConfigurationRegisters(configA, configB);
}

void BMS_SetUnderVoltageLimit(uint16_t voltageInMv) {
	uint8_t configA[6] = { 0 };
	uint8_t configB[6] = { 0 };

	// Get the up-to-date configuration registers
	BMS_GetConfigurationRegisters(configA, configB);
	// Convert voltage from mV to the 12-bit undervoltage limit value
	uint16_t voltageLimit = ((voltageInMv * 10) / 16) - 1;

	// Set the undervoltage limit (12-bit value)
	configA[1] = voltageLimit & 0xFF;
	configA[2] = (configA[2] & 0xF0) | ((voltageLimit >> 8) & 0x0F);

	// Write the new configuration back
	BMS_SetConfigurationRegisters(configA, configB);
}

/* Voltage Collection */
void BMS_GetVoltagesAll(uint16_t *voltages) {
	const uint16_t read_commands[] = {
	READ_CELL_VOLTAGE_A,
	READ_CELL_VOLTAGE_B,
	READ_CELL_VOLTAGE_C,
	READ_CELL_VOLTAGE_D,
	READ_CELL_VOLTAGE_E,
	READ_CELL_VOLTAGE_F };

	//TODO: Poll if registers are ready and return if not

	// Read the current Register Values
	for (int i = 0; i < 6; i++) {
		// init rx buffer
		uint8_t rxData[6] = { 0 };
		// Send command to read cell voltages for register A-F
		BMS_ReadRegister(read_commands[i], rxData, 6);

		// Assuming each register returns voltages of 3 cells (6 bytes)
		for (int j = 0; j < 3; j++) {
			voltages[i * 3 + j + 1] = (rxData[2 * j + 1] << 8) | rxData[2 * j];
		}
	}

	// Find the minimum and maximum cell voltages
	status.minVoltage = 0xFFFF;
	status.maxVoltage = 0;
	for (int cell = 0; cell < config.numberOfCells; cell++) {
		uint16_t voltage = status.cellVoltages[cell];
		if (voltage < status.minVoltage) {
			status.minVoltage = voltage;
		}
		if (voltage > status.maxVoltage) {
			status.maxVoltage = voltage;
		}
	}


	// Start adc conversion for all cells at the standard 7kHz
	BMS_SendCommand(START_ADC_CONVERSION_7KHZ, true);
	// As this function is called every 10ms and it takes <5ms to convert this will be ready by next call
}

/* Passive  Balancing */
void BMS_PassiveBalanceCells() {

    for (int cell = 0; cell < config.numberOfCells; cell++) {
        uint16_t voltage = status.cellVoltages[cell];  // in mV

        // If Voltage is below the minimum balancing voltage then don't balance
        if (voltage <= MIN_BALANCING_CELL_VOLTAGE) {
            BMS_SetCellDischarge(cell, false);
            continue;
        }

        if (status.cellVoltages[cell] > status.minVoltage + config.passiveBalanceThreshold) {
            BMS_SetCellDischarge(cell, true);
        } else {
            BMS_SetCellDischarge(cell, false);
        }
    }

    HAL_GPIO_WritePin(STM_AUX_2_GPIO_Port, STM_AUX_2_Pin, GPIO_PIN_RESET);
}

void BMS_SetCellDischargeMuteAll(bool mute) {
	uint8_t configA[6];
	uint8_t configB[3];

	// Read the current configuration
	BMS_GetConfigurationRegisters(configA, configB);

	// Update discharge cell states
	configA[4] = (status.cellPassiveBalancingFlags >> 1) & 0xFF;
	configA[5] = (configA[5] & 0xF0)
			| ((status.cellPassiveBalancingFlags >> 9) & 0x0F);
	configB[0] = (configB[0] & 0xF0)
			| ((status.cellPassiveBalancingFlags >> 13) & 0x0F);
	configB[1] = (configB[1] & 0xF8) | ((status.cellPassiveBalancingFlags & 0x01) << 2)
			| ((status.cellPassiveBalancingFlags >> 17) & 0x03);

	// Write the new configuration back
	BMS_SetConfigurationRegisters(configA, configB);
}

void BMS_SetCellDischargeAll() {
	// cell 1 to 12 A, 13 to 18 B
	uint8_t configA[6] = { 0 };
	uint8_t configB[6] = { 0 };

	// Get the up-to-date configuration registers
	BMS_GetConfigurationRegisters(configA, configB);

	// Set the bits for configA (Cells 1 to 12)
	configA[4] = (status.cellPassiveBalancingFlags & 0xFF);          				// Cells 1 to 8
	configA[5] = (configA[5] & 0xF0) | ((status.cellPassiveBalancingFlags >> 8) & 0x0F); // Cells 9 to 12 (4 bits)

	// Set the bits for configB (Cells 13 to 18)
	configB[0] = (configB[0] & 0xF0) | ((status.cellPassiveBalancingFlags >> 12) & 0x0F);	// Cells 13 to 16 (4 bits)
	configB[1] = (configB[1] & 0xFC) | ((status.cellPassiveBalancingFlags >> 16) & 0x03);	// Cells 17 and 18 (2 bits)

	// Write the new configuration back
	BMS_SetConfigurationRegisters(configA, configB);
}

void BMS_SetCellDischarge(uint8_t cell, bool discharge) {
	// cell 1 to 12 A, 13 to 18 B
	uint8_t configA[6] = { 0 };
	uint8_t configB[6] = { 0 };

	// Get the up to date configuration registers
	BMS_GetConfigurationRegisters(configA, configB);

	// Set the correct bit in the correct cell configuration register (see pg. 65 of datasheet)
	if (discharge) {
		status.cellPassiveBalancingFlags |= (1 << cell);
		if (cell <= 8) {
			// Cells 1 to 8
			configA[4] |= (1 << (cell - 1));
		} else if (cell <= 12) {
			// Cells 9 to 12
			configA[5] |= (1 << (cell - 9));
		} else if (cell <= 16) {
			// Cells 13 to 16
			configB[0] |= (1 << (cell - 9));
		} else {
			// Cells 17 and 18
			configB[1] |= (1 << (cell - 17));
		}
	} else {
		status.cellPassiveBalancingFlags &= ~(1 << cell);
		if (cell <= 8) {
			// Cells 1 to 8
			configA[4] &= ~(1 << (cell - 1));
		} else if (cell <= 12) {
			// Cells 9 to 12
			configA[5] &= ~(1 << (cell - 9));
		} else if (cell <= 16) {
			// Cells 13 to 16
			configB[0] &= ~(1 << (cell - 9));
		} else {
			// Cells 17 and 18
			configB[1] &= ~(1 << (cell - 17));
		}
	}

	// Write the new configuration back
	BMS_SetConfigurationRegisters(configA, configB);
}

/* Other BMS Functions */
uint32_t BMS_GetOverVoltageCells() {
	uint8_t auxiliaryD[6] = { 0 };
	uint8_t statusB[6] = { 0 };

	BMS_ReadRegister(READ_AUXILIARY_REGISTER_D, auxiliaryD, 6);
	BMS_ReadRegister(READ_STATUS_REGISTER_B, statusB, 6);

	uint32_t overVoltageCells = 0;
	// Extract CxOV bits from STBR2
	overVoltageCells |= ((statusB[2] >> 1) & 0x01) << 0;  // C1OV to bit 0
	overVoltageCells |= ((statusB[2] >> 3) & 0x01) << 1;  // C2OV to bit 1
	overVoltageCells |= ((statusB[2] >> 5) & 0x01) << 2;  // C3OV to bit 2
	overVoltageCells |= ((statusB[2] >> 7) & 0x01) << 3;  // C4OV to bit 3

	// Extract CxOV bits from STBR3
	overVoltageCells |= ((statusB[3] >> 1) & 0x01) << 4;  // C5OV to bit 4
	overVoltageCells |= ((statusB[3] >> 3) & 0x01) << 5;  // C6OV to bit 5
	overVoltageCells |= ((statusB[3] >> 5) & 0x01) << 6;  // C7OV to bit 6
	overVoltageCells |= ((statusB[3] >> 7) & 0x01) << 7;  // C8OV to bit 7

	// Extract CxOV bits from STBR4
	overVoltageCells |= ((statusB[4] >> 1) & 0x01) << 8;  // C9OV to bit 8
	overVoltageCells |= ((statusB[4] >> 3) & 0x01) << 9;  // C10OV to bit 9
	overVoltageCells |= ((statusB[4] >> 5) & 0x01) << 10; // C11OV to bit 10
	overVoltageCells |= ((statusB[4] >> 7) & 0x01) << 11; // C12OV to bit 11

	// Extract CxOV bits from ADVR4
	overVoltageCells |= ((auxiliaryD[4] >> 1) & 0x01) << 12;  // C13OV to bit 4
	overVoltageCells |= ((auxiliaryD[4] >> 3) & 0x01) << 13;  // C14OV to bit 5
	overVoltageCells |= ((auxiliaryD[4] >> 5) & 0x01) << 14;  // C15OV to bit 6
	overVoltageCells |= ((auxiliaryD[4] >> 7) & 0x01) << 15;  // C16OV to bit 7

	// Extract CxOV bits from ADVR5
	overVoltageCells |= ((auxiliaryD[5] >> 1) & 0x01) << 16;  // C17OV to bit 8
	overVoltageCells |= ((auxiliaryD[5] >> 3) & 0x01) << 17;  // C18OV to bit 9

	return overVoltageCells;
}

uint32_t BMS_GetUnderVoltageCells() {

	uint8_t auxiliaryD[6] = { 0 };
	uint8_t statusB[6] = { 0 };

	BMS_ReadRegister(READ_AUXILIARY_REGISTER_D, auxiliaryD, 6);
	BMS_ReadRegister(READ_STATUS_REGISTER_B, statusB, 6);

	uint32_t underVoltageCells = 0;
	// Extract CxOV bits from STBR2
	underVoltageCells |= ((statusB[2] >> 0) & 0x01) << 0;  // C1OV to bit 0
	underVoltageCells |= ((statusB[2] >> 2) & 0x01) << 1;  // C2OV to bit 1
	underVoltageCells |= ((statusB[2] >> 4) & 0x01) << 2;  // C3OV to bit 2
	underVoltageCells |= ((statusB[2] >> 6) & 0x01) << 3;  // C4OV to bit 3

	// Extract CxOV bits from STBR3
	underVoltageCells |= ((statusB[3] >> 0) & 0x01) << 4;  // C5OV to bit 4
	underVoltageCells |= ((statusB[3] >> 2) & 0x01) << 5;  // C6OV to bit 5
	underVoltageCells |= ((statusB[3] >> 4) & 0x01) << 6;  // C7OV to bit 6
	underVoltageCells |= ((statusB[3] >> 6) & 0x01) << 7;  // C8OV to bit 7

	// Extract CxOV bits from STBR4
	underVoltageCells |= ((statusB[4] >> 0) & 0x01) << 8;  // C9OV to bit 8
	underVoltageCells |= ((statusB[4] >> 2) & 0x01) << 9;  // C10OV to bit 9
	underVoltageCells |= ((statusB[4] >> 4) & 0x01) << 10; // C11OV to bit 10
	underVoltageCells |= ((statusB[4] >> 6) & 0x01) << 11; // C12OV to bit 11

	// Extract CxOV bits from ADVR4
	underVoltageCells |= ((auxiliaryD[4] >> 0) & 0x01) << 12;  // C13OV to bit 4
	underVoltageCells |= ((auxiliaryD[4] >> 2) & 0x01) << 13;  // C14OV to bit 5
	underVoltageCells |= ((auxiliaryD[4] >> 4) & 0x01) << 14;  // C15OV to bit 6
	underVoltageCells |= ((auxiliaryD[4] >> 6) & 0x01) << 15;  // C16OV to bit 7

	// Extract CxOV bits from ADVR5
	underVoltageCells |= ((auxiliaryD[5] >> 0) & 0x01) << 16;  // C17OV to bit 8
	underVoltageCells |= ((auxiliaryD[5] >> 2) & 0x01) << 17;  // C18OV to bit 9

	return underVoltageCells;
}

uint8_t BMS_GetWatchdogStatus() {
	return HAL_GPIO_ReadPin(PBIC_TIMEOUT_GPIO_Port, PBIC_TIMEOUT_Pin) == GPIO_PIN_SET;
}
