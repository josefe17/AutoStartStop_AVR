/*
 * DebugGPIO.c
 *
 * Created: 03/11/2023 12:47:48
 *  Author: josefe
 */ 

#include "DebugGPIO.h"

void initDebugGPIOInput()
{
	DDRB &= ~(1 << PB0);
	PORTB &= ~(1 << PB0);
}

void initDebugGPIOInputPullup()
{
	MCUCR &= ~(1 << PUD);
	DDRB &= ~(1 << PB0);
	PORTB |= (1 << PB0);
}

void initDebugGPIOOutput()
{
	DDRB |= (1 << PB0);
	PORTB &= ~(1 << PB0);
}

void initDebugGPIOOutputHigh()
{
	DDRB |= (1 << PB0);
	PORTB |= (1 << PB0);
}

void initDebugGPIOOutputLow()
{
	DDRB |= (1 << PB0);
	PORTB &= ~(1 << PB0);
}

uint8_t readDebugGPIO()
{
	return (PINB & (1 << PB0));
}

void writeDebugGPIO(uint8_t value)
{
	if (value)
	{
		PORTB |= (1 << PB0);
	}
	else
	{
		PORTB &= ~(1 << PB0);
	}
}