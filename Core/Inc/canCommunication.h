#ifndef INC_CANCOMMUNICATION_H_
#define INC_CANCOMMUNICATION_H_

#include <stdint.h>

/**
 * @brief Sends a CAN frame with the specified CAN ID and data.
 *
 * This function sends a CAN frame with the provided CAN ID and data. The frame length
 * specifies the number of bytes in the data array to be sent.
 *
 * @param canId The CAN ID for the frame.
 * @param data Pointer to the data array to be sent.
 * @param frameLength The length of the data array.
 */
void CAN_SendFrame(uint32_t canId, uint8_t *data, uint8_t frameLength);

/**
 * @brief Sends the status information over CAN.
 *
 * This function sends the status information including min and max voltages, max temperature,
 * and various flags over CAN. The data is packed into two 8-byte arrays.
 */
void CAN_SendStatus();

/**
 * @brief Sends the cell voltages over CAN.
 *
 * This function sends the cell voltages in CAN frames. Each frame contains the voltages of up to
 * four cells. The CAN ID is modified based on the index of the cells.
 */
void CAN_SendCellVoltages();


/**
 * @brief Sends the BMS configuration over CAN.
 *
 * This function sends the current BMS configuration parameters over CAN.
 */
void CAN_SendBmsConfiguration();

/**
 * @brief Sends the balance configuration over CAN.
 *
 * This function sends the current balance configuration parameters over CAN.
 */
void CAN_SendBalanceConfiguration();

/**
 * @brief Receives and sets the BMS configuration from CAN data.
 * 
 * This function extracts the BMS (Battery Management System) configuration
 * parameters from the received CAN data and sets them in the configuration
 * structure.
 * 
 * @param data Pointer to the received CAN data array.
 *             - data[0]: Number of cells.
 *             - data[1]: Number of thermistors.
 *             - data[2] & data[3]: Over voltage in mV (little-endian).
 *             - data[4] & data[5]: Under voltage in mV (little-endian).
 *             - data[6]: Shutdown temperature.
 *             - data[7]: Warning temperature.
 */
void CAN_ReceiveBmsConfiguration(uint8_t *data);

/**
 * @brief Receives and sets the balance configuration from CAN data.
 * 
 * This function extracts the balance configuration parameters from the
 * received CAN data and sets them in the configuration structure.
 * 
 * @param data Pointer to the received CAN data array.
 *             - data[0]: Passive balance threshold.
 *             - data[1]: Active balance threshold.
 *             - data[2]: Passive balance duty cycle.
 *             - data[3]: Active balance current.
 */
void CAN_ReceiveBalanceConfiguration(uint8_t *data);

/**
 * @brief Receives and sets the ISA current or ISA current counter from CAN data.
 * 
 * This function extracts either the ISA current or the ISA current counter from
 * the received CAN data based on the type parameter and sets the corresponding
 * value in the status structure.
 * 
 * @param type Specifies the type of data being received.
 *             - 0: ISA Current.
 *             - 1: ISA Current Counter.
 * @param data Pointer to the received CAN data array.
 *             - If type is 0:
 *               - data[0] & data[1]: ISA current (little-endian). TBC
 *             - If type is 1:
 *               - data[0], data[1], data[2], data[3]: ISA current counter (little-endian). TBC
 */
void CAN_ReceiveISACurrent(uint8_t type, uint8_t *data);

/**
 * @brief Requests remaining energies update from the Motec.
 *
 * This function sends a CAN frame to request the Motec to update the remaining energies.
 */
void CAN_RequestRemainingEnergies();

/**
 * @brief Processes the received remaining energy data for a cell.
 *
 * This function updates the remaining energy for a specific cell based on the received CAN data.
 *
 * @param data Pointer to the received CAN data array.
 */
void CAN_GetRemainingEnergy(uint8_t* data);

/**
 * @brief Sets the remaining energies for all cells.
 *
 * This function sends CAN frames to set the remaining energies for all 18 cells.
 */
void CAN_SetAllRemainingEnergies();

/**
 * @brief Sets the remaining energy for a specific cell.
 *
 * This function sends a CAN frame to set the remaining energy for a specific cell.
 *
 * @param cellNum The cell number (1-18).
 * @param remainingEnergy The remaining energy value for the cell.
 */
void CAN_SetRemainingEnergy(uint8_t cellNum, uint16_t remainingEnergy);

// TODO: The following functions are declared in the original header but not implemented in the C file
/**
 * @brief Gets the remaining energies.
 *
 * @param data Pointer to store the retrieved remaining energies data.
 */
void CAN_GetRemainingEnergies(uint16_t* data);

#endif /* INC_CANCOMMUNICATION_H_ */
