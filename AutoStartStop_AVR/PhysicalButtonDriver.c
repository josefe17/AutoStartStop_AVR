/*
 * PhysicalButtonDriver.c
 *
 * Created: 02/11/2023 20:23:44
 *  Author: josefe
 */ 


#include "PhysicalButtonDriver.h"

uint8_t readButtonRaw()
{
	return (PINB & (1 << PB2)); // Low enabled, button pressed
}