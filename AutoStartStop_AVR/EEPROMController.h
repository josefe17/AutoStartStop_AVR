/*
 * EEPROMController.h
 *
 * Created: 02/11/2023 21:39:02
 *  Author: josefe
 */ 


#ifndef EEPROMCONTROLLER_H_
#define EEPROMCONTROLLER_H_

#include <avr/io.h>

// Initializes the EEPROM controller
void initEEPROM();
// Read the EEPROM contains and update the flags
void readEEPROM(uint8_t* autoStarStopExpectedStatusPointer, uint8_t* switchOverrideModePointer);
// Requires the EEPROM to be written
void setEEPROMDirtyFlag();
// Write the flag contents to the EEPROM
uint8_t processEEPROM(uint8_t autoStarStopExpectedStatusValue, uint8_t switchOverrideModeValue);

#endif /* EEPROMCONTROLLER_H_ */