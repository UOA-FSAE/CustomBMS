#include "main.h"
#include "structs.h"
#include "flashStorage.h"
#include "canCommunication.h"

#define REMAINING_ENERGY_FLASH_START_ADDR 0x08007FA0 //TODO: Confirm this is appropriate
#define REMAINING_ENERGY_COUNT 18

extern BmsStatus status;

void FLASH_CheckForEnergyStorage() {
    FLASH_ReadFromFlash(REMAINING_ENERGY_FLASH_START_ADDR, status.cellRemainingEnergy, REMAINING_ENERGY_COUNT);

    // Check if all values are 0 (indicating a loss of memory)
    for (uint8_t i = 0; i < REMAINING_ENERGY_COUNT; i++) {
        if (status.cellRemainingEnergy[i] != 0 && status.cellRemainingEnergy[i] != -1) {
            return;  // Valid data found, return immediately
        }
    }

    // If we get here, all values were 0, so request new energies
    status.remainingEnergiesRequested = REMAINING_ENERGY_COUNT;
    CAN_RequestRemainingEnergies();

    while (status.remainingEnergiesRequested > 0) {
        // Hang here until all energies are received
		HAL_GPIO_TogglePin(CAN_ERROR_GPIO_Port, CAN_ERROR_Pin);
		HAL_Delay(100); // Delay To ensure Motec & rest of car ready
    }
	HAL_GPIO_WritePin(CAN_ERROR_GPIO_Port, CAN_ERROR_Pin, GPIO_PIN_RESET);

    // At this point, new energies should be available in the status struct
    // Write it to flash Now to ensure it's saved
	FLASH_SaveRemainingEnergies();
}


void FLASH_SaveRemainingEnergies() {
    FLASH_WriteToFlash(REMAINING_ENERGY_FLASH_START_ADDR, status.cellRemainingEnergy, REMAINING_ENERGY_COUNT);
}

void FLASH_WriteToFlash(uint32_t address, const int16_t* data, uint16_t size) {
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInitStruct;
    eraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInitStruct.PageAddress = address;
    eraseInitStruct.NbPages = 1;

    uint32_t pageError = 0;
    if (HAL_FLASHEx_Erase(&eraseInitStruct, &pageError) == HAL_OK) {
        for (uint16_t i = 0; i < size; i++) {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address + (i * sizeof(int16_t)), data[i]) != HAL_OK) {
                Error_Handler(); // :(
            }
        }
    }

    HAL_FLASH_Lock();
}

void FLASH_ReadFromFlash(uint32_t address, int16_t* data, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        data[i] = *(__IO int16_t*)(address + (i * sizeof(int16_t)));
    }
}
