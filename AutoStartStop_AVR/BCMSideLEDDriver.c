/*
 * BCMSideLEDDriver.c
 *
 * Created: 02/11/2023 20:22:12
 *  Author: josefe
 */ 

#include "BCMSideLEDDriver.h"
#include "TimerMillis.h"
#include "ButtonVariables.h"

uint8_t readBCMSideLEDLineRaw();

uint8_t lastBCMSideLEDLineState;
uint8_t currentBCMSideLEDLineState;
uint16_t debounceTimer;

void initBCMSideLED()
{
	// PB4 virtual LED input from BCM, with DDR = 0 and PORT = 0 (expected default values) (input, no pullup)
	DDRB &= ~(1 << PB4);
	PORTB &= ~(1 << PB4);
}

void initBCMSideLEDFilter()
{
	lastBCMSideLEDLineState = 1; // Initialized as released (inverted logic)
	currentBCMSideLEDLineState = 1;
}

uint8_t readBCMSideLEDLineFiltered()
{
	return currentBCMSideLEDLineState;
}

uint8_t readBCMSideLEDLineRaw()
{
	if (PINB & (1 << PB4))
	{
		return 0;
	}
	return 1;
}

void runBCMSideLEDFilter()
{
	uint8_t rawReading = readBCMSideLEDLineRaw();
	if (rawReading != lastBCMSideLEDLineState)
	{
		debounceTimer = readTimerMillis() + LED_LINE_DEBOUNCE_MILLIS;
	}
	lastBCMSideLEDLineState = rawReading;	
	if (checkDelayUntil(debounceTimer))
	{
		currentBCMSideLEDLineState = rawReading;
	}	
}
