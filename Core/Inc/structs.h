/*
 * statusStruct.h
 *
 *  Created on: Aug 5, 2024
 *      Author: Bcspa
 */

#ifndef INC_STATUSSTRUCT_H_
#define INC_STATUSSTRUCT_H_
#include <stdint.h>

/**
 * @brief Structure to hold the status of the Battery Management System (BMS).
 * 
 * This structure contains various parameters related to the current status
 * of the BMS, including cell voltages, temperatures, balancing information,
 * and error flags.
 */
typedef struct {
    // Cell Voltages
    uint16_t cellVoltages[20];       /** Array of cell voltages in millivolts. */
    uint16_t minVoltage;             /** Minimum cell voltage in millivolts. */
    uint16_t maxVoltage;             /** Maximum cell voltage in millivolts. */

    // SoC for segment
    float segmentCoulombCount;     /** Pack coulomb counter. Calculated on the STM*/

    // Current Info
    int32_t packCurrent;            /** Pack current in milliamps. */
    int32_t packCurrentCounter;     /** Pack current counter. From ISA Sensor */

    // Temperatures
    uint16_t cellTemperatures[8];    /** Array of cell temperatures in 0.1 degrees Celsius. */
    uint16_t minTemp;               /** Minimum cell temperature in 0.1 degrees Celsius. */
    uint16_t maxTemp;                /** Maximum cell temperature in 0.1 degrees Celsius. */
    uint16_t stmTemperature;         /** STM temperature in 0.1 degrees Celsius. */

    // Balancing Info
    uint8_t passiveBalancingActive : 1; /** Flag indicating if passive balancing is currently active. */
    uint32_t cellPassiveBalancingFlags;    /** Bitfield indicating which cells are passively balancing. */
    uint8_t numCellsBalancing : 6; /** Number of cells currently balancing. */

    // Error Flags
    uint8_t cellOverVoltage : 1;    /** Flag indicating a cell overvoltage condition. */
    uint8_t cellUnderVoltage : 1;   /** Flag indicating a cell undervoltage condition. */
    uint8_t cellShutdownTemperature : 1; /** Flag indicating a cell shutdown temperature condition. */
    uint8_t cellWarningTemperature : 1; /** Flag indicating a cell warning temperature condition. */
    uint8_t pbIcWatchdogTimeout : 1; /** Flag indicating a passive balancing IC watchdog timeout. */
} BmsStatus;

/**
 * @brief Structure to hold the configuration of the Battery Management System (BMS).
 * 
 * This structure contains various configuration parameters for the BMS, including
 * the number of cells, thermistors, voltage thresholds, temperature thresholds,
 * and balancing configurations.
 */
typedef struct {
    // Number of cells and thermistors
    uint8_t numberOfCells;           /** Number of cells the BMS will report. */
    uint8_t numberOfPopulatedCells;  /** Number of cells actually in the BMS. */
    uint8_t numberOfThermistors;     /** Number of thermistors in the BMS. */
    uint8_t populatedCells[7];       /** Array indicating which cells are populated. */

    // Shutdown and warning thresholds
    uint16_t overVoltageMV;          /** Over voltage threshold in millivolts. */
    uint16_t underVoltageMV;         /** Under voltage threshold in millivolts. */
    uint8_t shutdownTemperature;     /** Shutdown temperature threshold in degrees Celsius. */
    uint8_t warningTemperature;      /** Warning temperature threshold in degrees Celsius. */

    // Balancing Configuration
    uint8_t passiveBalanceEnabled : 1; /** Flag indicating if passive balancing is enabled. */
    uint8_t passiveBalanceThreshold; /** Passive balance threshold in millivolts. */
    uint8_t passiveBalanceDutyCycle; /** Passive balance duty cycle percentage. */
} BmsConfig;



#endif /* INC_STATUSSTRUCT_H_ */
