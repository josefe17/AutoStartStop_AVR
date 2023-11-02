/*
 * PhysicalLEDDriver.c
 *
 * Created: 02/11/2023 20:19:46
 *  Author: josefe
 */ 

#include "PhysicalLEDDriver.h"
#include "BCMSideLEDDriver.h"

void initPhysicalLED()
{
	// PB1 physical LED output, with DDR = 1 and PORT = 1 (output high to avoid pullup glitches)
	DDRB |= (1 << PB1);
	PORTB |= (1 << PB1);
}

void turnPhysicalLEDOff()
{
	PORTB &= ~(1 << PB1);
}

void turnPhysicalLEDOn()
{
	PORTB |= (1 << PB1);
}

void forwardBCMSideLEDStatusToPhysicalLED()
{
	if (readAutoStartStopCurrentStatus())
	{
		turnPhysicalLEDOff();
	}
	else
	{
		turnPhysicalLEDOn();
	}
}