#ifndef INC_FLASHSTORAGE_H_
#define INC_FLASHSTORAGE_H_

#include <stdint.h>

/**
 * @brief Check for stored segment capacity in flash and request new data if necessary
 *
 * This function reads the segment capacity from a predefined flash address. If the
 * value is invalid (zero or negative, indicating loss of memory or first boot), it
 * requests new capacity data via CAN and waits until the data is received.
 */
void FLASH_CheckForSegmentCapacity(void);

/**
 * @brief Save segment capacity to flash
 *
 * This function writes the segment capacity from the BmsStatus to a predefined
 * flash address.
 */
void FLASH_SaveSegmentCapacity(void);

/**
 * @brief Write float data to flash memory
 *
 * This function writes a float value to the specified flash address.
 * It handles flash unlocking, erasing, and locking operations.
 * The float is stored as a 32-bit word in flash memory.
 *
 * @param address Starting address in flash memory to write to
 * @param data Pointer to the float value to be written
 */
void FLASH_WriteToFlash(uint32_t address, const float* data);

/**
 * @brief Read float data from flash memory
 *
 * This function reads a float value from the specified flash address.
 * The value is read as a 32-bit word and converted back to float.
 *
 * @param address Starting address in flash memory to read from
 * @param data Pointer to float where the read data will be stored
 */
void FLASH_ReadFromFlash(uint32_t address, float* data);

#endif /* INC_FLASHSTORAGE_H_ */
