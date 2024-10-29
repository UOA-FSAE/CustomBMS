#ifndef INC_FLASHSTORAGE_H_
#define INC_FLASHSTORAGE_H_

#include <stdint.h>

/**
 * @brief Check for stored energy data in flash and request new data from the Motec if necessary
 *
 * This function reads energy data from a predefined flash address. If all
 * values are zero (indicating loss of memory or first boot), it requests
 * new energy data via CAN and waits until all data is received.
 *
 * @note The array should be at least REMAINING_ENERGY_COUNT elements long
 */
void FLASH_CheckForEnergyStorage();

/**
 * @brief Save remaining energy data to flash
 *
 * This function writes the energy data from the BmsStatus to a predefined flash address.
 *
 * @note The array should contain REMAINING_ENERGY_COUNT elements
 */
void FLASH_SaveRemainingEnergies();

/**
 * @brief Write int16_t data to flash memory
 *
 * This function writes an array of int16_t values to the specified flash address.
 * It handles flash unlocking, erasing, and locking operations.
 *
 * @param address Starting address in flash memory to write to
 * @param data Pointer to the int16_t array to be written
 * @param size Number of int16_t elements to write
 */
void FLASH_WriteToFlash(uint32_t address, const int16_t* data, uint16_t size);

/**
 * @brief Read int16_t data from flash memory
 *
 * This function reads an array of int16_t values from the specified flash address.
 *
 * @param address Starting address in flash memory to read from
 * @param data Pointer to the int16_t array where read data will be stored
 * @param size Number of int16_t elements to read
 */
void FLASH_ReadFromFlash(uint32_t address, int16_t* data, uint16_t size);


#endif /* INC_FLASHSTORAGE_H_ */
