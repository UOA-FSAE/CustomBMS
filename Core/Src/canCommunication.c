#include "config.h"
#include "can.h"
#include "canCommunication.h"
#include "structs.h"
#include <string.h>

extern CAN_HandleTypeDef hcan;
extern CAN_FilterTypeDef sFilterConfig;
extern uint32_t Tx_mailbox;

extern BmsConfig config;
extern BmsStatus status;

extern uint32_t cellDischargeFlags;

float totalEnergyChangeMah = 0.0f;

void CAN_SendFrame(uint32_t canId, uint8_t *data, uint8_t frameLength) {
	HAL_GPIO_WritePin(CAN_OK_GPIO_Port, CAN_OK_Pin, GPIO_PIN_SET);
	CAN_TxHeaderTypeDef Tx_header;
	Tx_header.StdId = canId;
	Tx_header.RTR = CAN_RTR_DATA;
	Tx_header.IDE = CAN_ID_STD;
	Tx_header.TransmitGlobalTime = DISABLE;
	Tx_header.DLC = frameLength;

	while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0) {
		;
	}

	HAL_CAN_AddTxMessage(&hcan, &Tx_header, data, &Tx_mailbox);
	HAL_GPIO_WritePin(CAN_OK_GPIO_Port, CAN_OK_Pin, GPIO_PIN_RESET);
}

void CAN_SendStatus() {
	uint8_t dataA[8] = { 0 };

	dataA[0] = (status.minVoltage >> 8) & 0xFF;
	dataA[1] = status.minVoltage & 0xFF;
	dataA[2] = (status.maxVoltage >> 8) & 0xFF;
	dataA[3] = status.maxVoltage & 0xFF;
	dataA[4] = (status.minTemp >> 8) & 0xFF;
	dataA[5] = status.minTemp & 0xFF;
	dataA[6] = (status.maxTemp >> 8) & 0xFF;
	dataA[7] = status.maxTemp & 0xFF;

	CAN_SendFrame(CAN_ID_STATUS_UPDATE_A, dataA, 8);

	uint8_t dataB[8] = { 0, };

	dataB[0] |= (status.passiveBalancingActive << 0); // Passive Balancing Active
	dataB[0] |= (status.cellOverVoltage << 1); // Cell Over Voltage
	dataB[0] |= (status.cellUnderVoltage << 2); // Cell Under Voltage
	dataB[0] |= (status.cellShutdownTemperature << 3); // Cell Shutdown Temperature
	dataB[0] |= (status.cellWarningTemperature << 4); // Cell Warning Temperature
	dataB[0] |= (status.pbIcWatchdogTimeout << 5); // PB IC Watchdog Timeout

	dataB[1] = (status.cellPassiveBalancingFlags >> 1) & 0xFF; 	// Cells 1 to 8
	dataB[2] = ((status.cellPassiveBalancingFlags >> 1) >> 8) & 0xFF;// Cells 9 to 16
	dataB[3] = ((status.cellPassiveBalancingFlags >> 1) >> 16) & 0x03; // Cells 17 and 18

	CAN_SendFrame(CAN_ID_STATUS_UPDATE_B, dataB, 4);
}

void CAN_SendCellVoltages() {
	uint8_t data[8] = { 0 };

	for (uint16_t i = 1; i < config.numberOfCells; i += 4) {
		// If NUM_CELLS > 20 will overlap into temp IDs
		// Pack 4 cell voltages into the data array
		data[0] = (status.cellVoltages[i] >> 8) & 0xFF;
		data[1] = status.cellVoltages[i] & 0xFF;
		data[2] = (status.cellVoltages[i + 1] >> 8) & 0xFF;
		data[3] = status.cellVoltages[i + 1] & 0xFF;
		data[4] = (status.cellVoltages[i + 2] >> 8) & 0xFF;
		data[5] = status.cellVoltages[i + 2] & 0xFF;
		data[6] = (status.cellVoltages[i + 3] >> 8) & 0xFF;
		data[7] = status.cellVoltages[i + 3] & 0xFF;

		CAN_SendFrame(CAN_ID_CELL_VOLTAGES_BASE + (i / 4), data, 8);
	}
}
void CAN_SendBmsConfiguration() {
	uint8_t data[8] = { 0 };

	data[0] = config.numberOfPopulatedCells;
	data[1] = config.numberOfThermistors;
	data[2] = (config.overVoltageMV >> 8) & 0xFF;
	data[3] = config.overVoltageMV & 0xFF;
	data[4] = (config.underVoltageMV >> 8) & 0xFF;
	data[5] = config.underVoltageMV & 0xFF;
	data[6] = config.shutdownTemperature;
	data[7] = config.warningTemperature;

	CAN_SendFrame(CAN_ID_SEND_BMS_CONFIGURATION, data, 8);
}

void CAN_SendBalanceConfiguration() {
	uint8_t data[8] = { 0 };

	data[0] = (config.passiveBalanceEnabled << 0);
	data[1] = config.passiveBalanceThreshold;
	data[2] = config.passiveBalanceDutyCycle;

	CAN_SendFrame(CAN_ID_SEND_BALANCE_CONFIGURATION, data, 8);
}

void CAN_ReceiveBmsConfiguration(uint8_t *data) {
	if (data[0] >= 0 && data[0] < ABSMAX_POPULATED_CELLS) {
		config.numberOfPopulatedCells = data[0];
	}
	if (data[1] >= 0 && data[1] < ABSMAX_THERMISTOR_COUNT) {
		config.numberOfThermistors = data[1];
	}
	uint16_t newOverVoltage = data[3] | (data[2] << 8);
	if (newOverVoltage >= ABSMIN_CELL_VOLTAGE
			&& newOverVoltage < ABSMAX_CELL_VOLTAGE) {
		config.overVoltageMV = newOverVoltage;
	}
	uint16_t newUnderVoltage = data[5] | (data[4] << 8);
	if (newUnderVoltage >= ABSMIN_CELL_VOLTAGE
			&& newUnderVoltage < config.overVoltageMV) {
		config.underVoltageMV = newUnderVoltage;
	}
	if (data[6] >= 0 && data[6] < ABSMAX_TEMPERATURE) {
		config.shutdownTemperature = data[6];
	}
	if (data[7] >= 0 && data[7] < ABSMAX_TEMPERATURE) {
		config.warningTemperature = data[7];
	}
}

void CAN_ReceiveBalanceConfiguration(uint8_t *data) {
	config.passiveBalanceEnabled = (data[0] & 0b01);
	config.passiveBalanceThreshold = data[1];
	config.passiveBalanceDutyCycle = data[3];
}

void CAN_RequestRemainingEnergies() {
	uint8_t data[1] = { 0 };

	CAN_SendFrame(CAN_ID_REMAINING_ENERGIES, data, 1);
}

void CAN_GetRemainingEnergy(uint8_t* data) {
	// data sent in mAh
	status.segmentCoulombCount = (float) (data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24)) / 1000.0f;
}

void CAN_SetRemainingEnergy(uint32_t remainingEnergyMah) {
	uint8_t data[4] = { 0 };

	data[0] = remainingEnergyMah & 0xFF;
	data[1] = (remainingEnergyMah >> 8) & 0xFF;
	data[2] = (remainingEnergyMah >> 16) & 0xFF;
	data[3] = (remainingEnergyMah >> 24) & 0xFF;

	CAN_SendFrame(CAN_ID_REMAINING_ENERGIES, data, 4);
}
