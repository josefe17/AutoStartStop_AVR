/*
 * EEPROMController.c
 *
 * Created: 02/11/2023 21:39:20
 *  Author: josefe
 */ 

#include "EEPROMController.h"
#include <avr/eeprom.h>
#include "ButtonVariables.h"

// Compute the number of 1's in a byte
uint8_t countOnes(uint8_t number);

// Set if there were any changes to the NVM contents and writing is required
uint8_t markEEPROMForUpdate;


void initEEPROM()
{
	markEEPROMForUpdate = 0;
}

void readEEPROM(uint8_t* autoStarStopExpectedStatusPointer, uint8_t* switchOverrideModePointer)
{
	uint8_t value = eeprom_read_byte((uint8_t*) EEPROM_ADDRESS);
	uint8_t lowerNibble = value & 0x0F;
	uint8_t upperNibble = (value >> 4) & 0x0F;
	if (countOnes(lowerNibble) > 2)
	{
		(*autoStarStopExpectedStatusPointer) = 1;
	}
	else
	{
		(*autoStarStopExpectedStatusPointer) = 0;
	}
	if (countOnes(upperNibble) > 2)
	{
		(*switchOverrideModePointer) = 1;
	}
	else
	{
		(*switchOverrideModePointer) = 0;
	}
}

void setEEPROMDirtyFlag()
{
	markEEPROMForUpdate = 1;
}

void processEEPROM(uint8_t autoStarStopExpectedStatusValue, uint8_t switchOverrideModeValue)
{
	uint8_t value = 0;
	if (markEEPROMForUpdate && (eeprom_is_ready()))
	{
		if (autoStarStopExpectedStatusValue)
		{
			value |= 0x0F;
		}
		if (switchOverrideModeValue)
		{
			value |= 0xF0;
		}
		eeprom_update_byte((uint8_t*) EEPROM_ADDRESS, value);
		markEEPROMForUpdate = 0;
	}
}

uint8_t countOnes(uint8_t number)
{
	uint8_t count = 0;
	for (uint8_t index = 0; index < 7; ++index)
	{
		if (number & (1 << index))
		{
			++count;
		}
	}
	return count;
}