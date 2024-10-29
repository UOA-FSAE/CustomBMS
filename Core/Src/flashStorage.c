#include "main.h"
#include "structs.h"
#include "flashStorage.h"
#include "canCommunication.h"

#define SEGMENT_CAPACITY_FLASH_ADDR 0x08007FA0 // TODO: Confirm this is appropriate

extern BmsStatus status;

void FLASH_CheckForSegmentCapacity() {
    float capacity;
    FLASH_ReadFromFlash(SEGMENT_CAPACITY_FLASH_ADDR, &capacity);

    // Check if value is valid (not 0 or -1)
    if (capacity > 0.0f) {
        status.segmentCoulombCount = capacity;
        return;
    }

    status.segmentCoulombCount = -12.34f;
    // If we get here, value was invalid, so request new capacity
    CAN_RequestRemainingEnergies();

    while (status.segmentCoulombCount == -12.34f) {
        // Hang here until capacity is received
        HAL_GPIO_TogglePin(CAN_ERROR_GPIO_Port, CAN_ERROR_Pin);
        HAL_Delay(100); // Delay To ensure Motec & rest of car ready
    }
    HAL_GPIO_WritePin(CAN_ERROR_GPIO_Port, CAN_ERROR_Pin, GPIO_PIN_RESET);

    // At this point, new capacity should be available in the status struct
    // Write it to flash now to ensure it's saved
    FLASH_SaveSegmentCapacity();
}

void FLASH_SaveSegmentCapacity() {
    FLASH_WriteToFlash(SEGMENT_CAPACITY_FLASH_ADDR, &status.segmentCoulombCount);
}

void FLASH_WriteToFlash(uint32_t address, const float* data) {
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInitStruct;
    eraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInitStruct.PageAddress = address;
    eraseInitStruct.NbPages = 1;

    uint32_t pageError = 0;
    if (HAL_FLASHEx_Erase(&eraseInitStruct, &pageError) == HAL_OK) {
        // Write float as 32-bit word
        uint32_t floatAsInt;
        memcpy(&floatAsInt, data, sizeof(float));

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, floatAsInt) != HAL_OK) {
            Error_Handler();
        }
    }

    HAL_FLASH_Lock();
}

void FLASH_ReadFromFlash(uint32_t address, float* data) {
    // Read 32-bit word and convert back to float
    uint32_t storedValue = *(__IO uint32_t*)(address);
    memcpy(data, &storedValue, sizeof(float));
}
