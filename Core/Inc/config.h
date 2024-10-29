/*
 * config.h
 *
 *  Created on: Jul 12, 2024
 *      Author: Bcspa
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

// Uncomment to cycle cells (1, 2, 3, 4, 5...) when passive balancing disabled
//#define CYCLE_CELL_DISCHARGE
//#define CYCLE_CELL_CHARGE

#define DISCHARGE_RESISTANCE 					23.5

/* Absolute Maximum configurations */
#define ABSMAX_POPULATED_CELLS					18
#define ABSMAX_THERMISTOR_COUNT					8
#define ABSMAX_CELL_VOLTAGE						43500
#define ABSMIN_CELL_VOLTAGE						30000
#define ABSMAX_TEMPERATURE						70
#define MIN_BALANCING_CELL_VOLTAGE				32000
#define MAX_BALANCING_CELL_VOLTAGE				42000

/* CAN FRAME DEFINITIONS */
#define CAN_SEGMENT_NUMBER 0 	/// used to modify the CAN Frames to be identifiable by segment (0 indexed)
#define CAN_ID_STATUS_UPDATE_A 					0x200 + (CAN_SEGMENT_NUMBER << 4) // 8 byte frame giving status update
#define CAN_ID_STATUS_UPDATE_B					0x201 + (CAN_SEGMENT_NUMBER << 4) // 8 byte frame giving status update
#define CAN_ID_CELL_VOLTAGES_BASE 				0x202 + (CAN_SEGMENT_NUMBER << 4) // (Assuming 18 cells will use 0x201 - 0x205)
#define CAN_ID_CELL_TEMPERATURES_BASE 			0x207 + (CAN_SEGMENT_NUMBER << 4) // (Assuming 9 thermistors will use 0x206 - 0x208)
#define CAN_ID_BALANCING_INFORMATION 			0x209 + (CAN_SEGMENT_NUMBER << 4) // (Assuming 9 thermistors will use 0x206 - 0x208)
#define CAN_ID_SEND_BMS_CONFIGURATION 			0x20A + (CAN_SEGMENT_NUMBER << 4) //
#define CAN_ID_RECEIVE_BMS_CONFIGURATION 		0x20B + (CAN_SEGMENT_NUMBER << 4) //TODO: make unrelated to segment number
#define CAN_ID_SEND_BALANCE_CONFIGURATION 		0x20C + (CAN_SEGMENT_NUMBER << 4) //
#define CAN_ID_RECEIVE_BALANCE_CONFIGURATION 	0x20D + (CAN_SEGMENT_NUMBER << 4) //TODO: make unrelated to segment number
#define CAN_ID_PING_DATA						0x20E + (CAN_SEGMENT_NUMBER << 4)
#define CAN_ID_REMAINING_ENERGIES				0x20F + (CAN_SEGMENT_NUMBER << 4)

#define CAN_ID_ISA_CURRENT 0x521
#define CAN_ID_ISA_CURRENT_COUNTER 0x527
#endif /* INC_CONFIG_H_ */
