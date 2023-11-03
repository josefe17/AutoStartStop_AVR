/*
 * PhysicalLEDDriver.c
 *
 * Created: 02/11/2023 20:19:46
 *  Author: josefe
 */ 

#include "PhysicalLEDDriver.h"
#include "BCMSideLEDDriver.h"
#include "TimerMillis.h"

uint8_t readPhysicalLEDRawStatus();

uint16_t LEDBlinkTimer;

uint8_t threeBlinksSequenceStatus;
uint8_t longBlinkSequenceStatus;
uint8_t pendingThreeBlinksFlag;
uint8_t pendingLongBlinkFlag;

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

uint8_t readPhysicalLEDRawStatus()
{
	return ((PORTB >> PB1) & 1);
}

void forwardBCMSideLEDStatusToPhysicalLED()
{
	if (readBCMSideLEDLineFiltered())
	{
		turnPhysicalLEDOff();
	}
	else
	{
		turnPhysicalLEDOn();
	}
}

void initPhysicalLEDBlinkController()
{
	threeBlinksSequenceStatus = 0;
	longBlinkSequenceStatus = 0;
	pendingThreeBlinksFlag = 0;
	pendingLongBlinkFlag = 0;
}

void requestPhysicalLEDThreeBlinks()
{
	pendingThreeBlinksFlag = 1;
}

void requestPhysicalLEDLongBlink()
{
	pendingLongBlinkFlag = 0;	
}

uint8_t isPhysicalLEDBlinkingSequenceRunning()
{
	return (threeBlinksSequenceStatus + longBlinkSequenceStatus) > 0;
}

void runPhysicalLEDBlinks()
{
	// If a three blinks is pending and no sequence is running
	if (pendingThreeBlinksFlag && threeBlinksSequenceStatus == 0 && longBlinkSequenceStatus == 0)
	{
		// Clear flag and start it
		pendingThreeBlinksFlag = 0;
		threeBlinksSequenceStatus = 3;			
		turnPhysicalLEDOn();
		LEDBlinkTimer = readTimerMillis() + PHYSICALLEDTHREEBLINKSDELAY_MS;
	}
	// If a three blinks is pending and no sequence is running
	if (pendingLongBlinkFlag && threeBlinksSequenceStatus == 0 && longBlinkSequenceStatus == 0)
	{
		// Clear flag and start it
		pendingLongBlinkFlag = 0;
		longBlinkSequenceStatus = 1;
		turnPhysicalLEDOn();
		LEDBlinkTimer = readTimerMillis() + PHYSICALLEDLONGBLINKDELAY_MS;
	}
	// If the three LEDs sequence is running
	if (threeBlinksSequenceStatus > 0)
	{
		// If timeout
		if (checkDelayUntil(LEDBlinkTimer))
		{
			// If LED is off, a cycle has finished, so decrement counter
			if (readPhysicalLEDRawStatus() == 0)
			{
				--threeBlinksSequenceStatus;
				// If there are cycles pending, start a new one
				if (threeBlinksSequenceStatus > 0)
				{
					turnPhysicalLEDOn();
					LEDBlinkTimer = readTimerMillis() + PHYSICALLEDTHREEBLINKSDELAY_MS;
				}
			}
			else
			{
				// If LED is on, we're half a cycle, so perform the other half (LED off)
				turnPhysicalLEDOff();
				LEDBlinkTimer = readTimerMillis() + PHYSICALLEDTHREEBLINKSDELAY_MS;
			}
		}
	}
	// Same for long blink
	if (longBlinkSequenceStatus > 0)
	{
		if (checkDelayUntil(LEDBlinkTimer))
		{
			if (readPhysicalLEDRawStatus() == 0)
			{
				--longBlinkSequenceStatus;
				if (longBlinkSequenceStatus > 0)
				{
					turnPhysicalLEDOn();
					LEDBlinkTimer = readTimerMillis() + PHYSICALLEDLONGBLINKDELAY_MS;
				}
			}
			else
			{
				turnPhysicalLEDOff();
				LEDBlinkTimer = readTimerMillis() + PHYSICALLEDLONGBLINKDELAY_MS;
			}
		}
	}	
}