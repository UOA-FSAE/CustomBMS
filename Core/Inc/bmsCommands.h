#ifndef INC_BMSCOMMANDS_H_
#define INC_BMSCOMMANDS_H_
#include "stdint.h"
#include "stdbool.h"

// Command Definitions

// ADC Related Commands
#define CLEAR_ADC_VOLTAGE_REGISTERS 0x0711
#define START_ADC_CONVERSION_7KHZ 0x0360
#define START_ADC_CONVERSION_2KHZ 0x03E0
#define START_SUM_OF_ALL_CELLS 0x0567
// CMD[10:9]  = 01 	(Fixed)
// CMD[8:7]   = 10	(MD1=0 MD1=1: 7 kHz mode (normal)) (2Kz is [1,1])
// CMD[6:5]   = 11 	(Fixed)
// CMD[4]     = 0	(DCP 0: Discharge Not Permitted)
// CMD[3]     = 0	(Fixed)
// CMD[2:0]   = 000	(CH=000:Channel set to all channels)

// Command to read cell voltages
#define READ_CELL_VOLTAGE_A 0x0004 // 0b0100 (Cells 1,2,3)
#define READ_CELL_VOLTAGE_B 0x0006 // 0b0110 (Cells 4,5,6)
#define READ_CELL_VOLTAGE_C 0x0008 // 0b1000 (Cells 7,8,9)
#define READ_CELL_VOLTAGE_D 0x000A // 0b1010 (Cells 10,11,12)
#define READ_CELL_VOLTAGE_E 0x0009 // 0b1001 (Cells 13,14,15)
#define READ_CELL_VOLTAGE_F 0x000B // 0b1011 (Cells 16,17,18)

//Set Discharge is WRCFGA/B
#define WRITE_CONFIG_REGISTER_A 0x0001
#define READ_CONFIG_REGISTER_A 0x0002
// Byte 0 [7:3] = GPIO [5:1]
// Byte 0 [2]   = REFON (Enable Reference )
// Byte 0 [1]	= DTEN (Discharge timer enable)
// Byte 0 [0]	= ADCOPT (ADC mode select)
// Byte 1 [7:0]	= VUV [7:0] (under voltage limit)
// Byte 2 [3:0] = VUV [11:8]
// Byte 2 [7:4] = VOV [3:0] (over voltage limit)
// Byte 3 [7:0] = VOV [11:4]
// Byte 4 [7:0] = DCC [8:1] (Discharge cell X)
// Byte 5 [3:0] = DCC [12:9]
// Byte 5 [7:4] = DCTO [3:0] (Discharge timer config)


#define WRITE_CONFIG_REGISTER_B 0x0024
#define READ_CONFIG_REGISTER_B 0x0026
// Byte 0 [7:4] = DCC [16:13]
// Byte 0 [3:0} = GPIO [9:6]
// Byte 1 [7] = MUTE (Disable S pins (i.e. for sampling voltages)
// Byte 1 [6] = FDRF (Force digital redundancy failure)
// Byte 1 [5:4] = PS [1:0] (Digial redundancy path select
// Byte 1 [3] = DTMEN = (Enable discharge timer monitor)
// Byte 1 [2:0] = DCC [0, 18:17]

//used to get over/under-voltage cell flags
#define READ_AUXILIARY_REGISTER_D 0x000F
#define READ_STATUS_REGISTER_B 0x0012 

// Low Level Function Prototypes
/**
 * @brief Sends a command to the BMS.
 * @param command The command to send.
 * @param toggleCS Whether to toggle the chip select line.
 */
void BMS_SendCommand(uint16_t command, bool toggleCS);

/**
 * @brief Writes data to a BMS register.
 * @param command The command to write.
 * @param data Pointer to the data to write.
 * @param dataSize Size of the data to write.
 */
void BMS_WriteRegister(uint16_t command, uint8_t* data, uint8_t dataSize);

/**
 * @brief Reads data from a BMS register.
 * @param command The command to read.
 * @param data Pointer to the buffer to store the read data.
 * @param dataSize Size of the data to read.
 */
void BMS_ReadRegister(uint16_t command, uint8_t* data, uint8_t dataSize);

// High Level Function Prototypes
/**
 * @brief Gets the voltages of all cells.
 * @param voltages Pointer to the array to store the cell voltages.
 */
void BMS_GetVoltagesAll(uint16_t* voltages);

/**
 * @brief Initializes the configuration registers with default values.
 */
void BMS_InitConfigRegisters();

/**
 * @brief Sets the over-voltage limit for cells.
 * @param voltageInMv The over-voltage limit in millivolts.
 */
void BMS_SetOverVoltageLimit(uint16_t voltageInMv);

/**
 * @brief Sets the under-voltage limit for cells.
 * @param voltageInMv The under-voltage limit in millivolts.
 */
void BMS_SetUnderVoltageLimit(uint16_t voltageInMv);

/**
 * @brief Balances the cells of a by discharging cells with higher voltages.
 *
 * This function identifies the cell with the minimum voltage and discharges cells that have voltages
 * higher than the minimum voltage by a confi threshold threshold. This function should be called periodically
 * to ensure that passive balancing is correctly operating.
 *
 * @note `BMS_SetCellDischarge(uint8_t cell, bool enable)`: An external function that controls the discharge state of a specific cell.
 */
void BMS_PassiveBalanceCells();

/*
 *
 */
void BMS_SetCellCharge(uint8_t cell, bool enable);

/**
 * @brief Mutes or unmutes the discharge for all cells.
 * @param mute True to mute, false to unmute.
 */
void BMS_SetCellDischargeMuteAll(bool mute);

/**
 * @brief Sets the discharge state for all cells based on `status.cellPassiveBalancingFlags`
 */
void BMS_SetCellDischargeAll();

/**
 * @brief Sets the discharge state for a specific cell.
 * @param cell The cell number to set the discharge state for.
 * @param discharge True to enable discharge, false to disable.
 */
void BMS_SetCellDischarge(uint8_t cell, bool discharge);

/**
 * @brief Sets the configuration registers.
 * @param configA Pointer to the configuration data for register A.
 * @param configB Pointer to the configuration data for register B.
 */
void BMS_SetConfigurationRegisters(uint8_t* configA, uint8_t* configB);

/**
 * @brief Gets the configuration registers.
 * @param configA Pointer to the buffer to store the configuration data for register A.
 * @param configB Pointer to the buffer to store the configuration data for register B.
 */
void BMS_GetConfigurationRegisters(uint8_t* configA, uint8_t* configB);

/**
 * @brief Gets the cells with over-voltage.
 * @return A bitmask indicating the over-voltage status of each cell.
 */
uint32_t BMS_GetOverVoltageCells();

/**
 * @brief Gets the cells with under-voltage.
 * @return A bitmask indicating the under-voltage status of each cell.
 */
uint32_t BMS_GetUnderVoltageCells();

/**
 * @brief (TODO) Gets the temperatures of all cells.
 */
void BMS_GetTemperatureAll();

/**
 * @brief Gets the watchdog status.
 * @return The status of the watchdog.
 */
uint8_t BMS_GetWatchdogStatus();

// TODO: Below functions
void BMS_GetTemperatureAll();

#endif /* INC_BMSCOMMANDS_H_ */
