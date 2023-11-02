/*
 * PhysicalLEDDriver.c
 *
 * Created: 02/11/2023 20:19:46
 *  Author: josefe
 */ 

#include "PhysicalLEDDriver.h"
#include <avr/io.h>

void initFeedbackLED()
{
	// PB1 physical LED output, with DDR = 1 and PORT = 1 (output high to avoid pullup glitches)
	DDRB |= (1 << PB1);
	PORTB |= (1 << PB1);
}

void turnLEDOff()
{
	PORTB &= ~(1 << PB1);
}

void turnLEDOn()
{
	PORTB |= (1 << PB1);
}