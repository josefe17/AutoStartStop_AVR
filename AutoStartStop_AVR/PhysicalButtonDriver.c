/*
 * PhysicalButtonDriver.c
 *
 * Created: 02/11/2023 20:23:44
 *  Author: josefe
 */ 


#include "PhysicalButtonDriver.h"


// PB2 physical button input, with DDR = 0 (expected default value) and PORT = 1 (input pullup)
void initPhysicalButton()
{
	MCUCR &= ~(1 << PUD);
	DDRB &= ~(1 << PB2);
	PORTB |= (1 << PB2);
}

uint8_t readPhysicalButtonRaw()
{
	return (PINB & (1 << PB2)); // Low enabled, button pressed
}