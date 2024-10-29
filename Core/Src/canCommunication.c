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
	dataA[4] = (status.maxTemp >> 8) & 0xFF;
	dataA[5] = status.maxTemp & 0xFF;

	CAN_SendFrame(CAN_ID_STATUS_UPDATE_A, dataA, 8);

	uint8_t dataB[8] = { 0, };

	dataB[0] |= (status.passiveBalancingActive << 0); // Passive Balancing Active
	dataB[0] |= (status.cellOverVoltage << 1); // Cell Over Voltage
	dataB[0] |= (status.cellUnderVoltage << 2); // Cell Under Voltage
	dataB[0] |= (status.cellShutdownTemperature << 3); // Cell Shutdown Temperature
	dataB[0] |= (status.cellWarningTemperature << 4); // Cell Warning Temperature
	dataB[0] |= (status.pbIcWatchdogTimeout << 5); // PB IC Watchdog Timeout
	dataB[0] |= (status.abIcWatchdogTimeout << 6); // AB IC Watchdog Timeout

	dataB[1] = (status.cellPassiveBalancingFlags >> 1) & 0xFF; 	// Cells 1 to 8
	dataB[2] = ((status.cellPassiveBalancingFlags >> 1) >> 8) & 0xFF;// Cells 9 to 16
	dataB[3] = ((status.cellPassiveBalancingFlags >> 1) >> 16) & 0x03; // Cells 17 and 18

	dataB[4] = (status.cellActiveBalancingFlags >> 1) & 0xFF; 	// Cells 1 to 8
	dataB[5] = ((status.cellActiveBalancingFlags >> 1) >> 8) & 0xFF;// Cells 9 to 16
	dataB[6] = ((status.cellActiveBalancingFlags >> 1) >> 16) & 0x03; // Cells 17 and 18

	CAN_SendFrame(CAN_ID_STATUS_UPDATE_B, dataB, 7);
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

	data[0] = (config.passiveBalanceEnabled << 0)
			| (config.activeBalanceEnabled << 1);
	data[1] = config.passiveBalanceThreshold;
	data[2] = config.activeBalanceThreshold;
	data[3] = config.passiveBalanceDutyCycle;
	data[4] = config.activeBalanceCurrent;

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
	config.activeBalanceEnabled = (data[0] & 0x02) >>1;
	config.passiveBalanceThreshold = data[1];
	config.activeBalanceThreshold = data[2];
	config.passiveBalanceDutyCycle = data[3];
	config.activeBalanceCurrent = data[4];
}

void CAN_ReceiveISACurrent(uint8_t type, uint8_t *data) {
    if (type == 0) {
        // ISA Current
    	status.packCurrent = data[5] | (data[4] << 8) | (data[3] << 16) | (data[2] << 24);
    	if (status.cellActiveBalancingFlags) {
    		status.packCurrent += 257;
    	} else if (HAL_GPIO_ReadPin(AB_DRIVE_EN_GPIO_Port, AB_DRIVE_EN_Pin)) {
    		status.packCurrent += 57;
    	}

        float energyChangeMah = (float)status.packCurrent * (5.0f / 3600000.0f) * 1.054f;  // mAh change in 5ms
        totalEnergyChangeMah += energyChangeMah;

        int intEnergyChange = (int)totalEnergyChangeMah;
        if (intEnergyChange != 0) {
            // Update each cell's remaining energy
            for (int i = 1; i <= 18; i++) {
                status.cellRemainingEnergy[i] -= intEnergyChange;
            }
            totalEnergyChangeMah -= intEnergyChange;  // Keep the fractional part
        }

        // Update pack coulomb count (in As)
        status.packCoulombCount += (float)status.packCurrent / 200000.0f;
    } else if (type == 1) {
        // ISA Current Counter
    	status.packCurrentCounter = data[5] | (data[4] << 8) | (data[3] << 16) | (data[2] << 24);
    }
}

void CAN_RequestRemainingEnergies() {
	uint8_t data[3] = { 0 };

	CAN_SendFrame(CAN_ID_REMAINING_ENERGIES, data, 3);
	// Sends a blank 0,0,0 t request the motec update the remaining energies
}

void CAN_GetRemainingEnergy(uint8_t* data) {
	if (status.remainingEnergiesRequested == 0) {
		if (data[0] == 0 && data[1] == 0 && data[2] == 0)
			status.remainingEnergiesRequested = 18;
		// We're not expecting an updated energy so return
		return;
	}
	status.remainingEnergiesRequested--;
	status.cellRemainingEnergy[data[0]] = ((uint16_t)data[2] | (uint16_t)data[1] << 8);
}

void CAN_SetAllRemainingEnergies() {
	// 18 Cells
	for (uint8_t cell = 1; cell < 19; cell++) {
		CAN_SetRemainingEnergy(cell, status.cellRemainingEnergy[cell]);
	}
}
void CAN_SetRemainingEnergy(uint8_t cellNum, uint16_t remainingEnergy) {
	uint8_t data[3] = { 0 };
	data[0] = cellNum;
	data[1] = (remainingEnergy >> 8) & 0xFF;
	data[2] = (remainingEnergy & 0xFF);
	CAN_SendFrame(CAN_ID_REMAINING_ENERGIES, data, 3);
	// Updates the Energ
}
