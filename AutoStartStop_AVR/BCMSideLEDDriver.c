/*
 * BCMSideLEDDriver.c
 *
 * Created: 02/11/2023 20:22:12
 *  Author: josefe
 */ 

#include "BCMSideLEDDriver.h"

void initBCMSideLED()
{
	// PB4 virtual LED input from BCM, with DDR = 0 and PORT = 0 (expected default values) (input, no pullup)
	DDRB &= ~(1 << PB4);
	PORTB &= ~(1 << PB4);
}

uint8_t readAutoStartStopCurrentStatus()
{
	if (PINB & (1 << PB4))
	{
		return 0;
	}
	return 1;
}
