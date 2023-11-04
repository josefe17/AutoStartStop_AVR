/*
 * BCMSideButtonDriver.c
 *
 * Created: 02/11/2023 20:23:05
 *  Author: josefe
 */ 

#include "BCMSideButtonDriver.h"
#include "TimerMillis.h"
#include "ButtonVariables.h"
#include "PhysicalButtonDriver.h"

// Pulse output FSM state
enum ButtonFSMStates pulseButtonState;
// Counter of the number of pending pulse requests
uint8_t pulseRequests;
// Flag for aborting ogoing pulses
uint8_t abortOngoingPulseRequestFlag;
// Time instant when the simulated button depression ends
uint16_t pulseDurationEndingTime;
// Time instant when the simulated button settling time ends
uint16_t pulseSettleEndingTime;
	
// Checks if the abort flag is set and clears it
uint8_t checkAbortOngoingPulseRequestFlag();

void initBCMSideButtonLine()
{
	// PB3 virtual button output to BCM, with DDR = 1 and PORT = 0 (expected default value) (output, no pullup)
	DDRB |= (1 << PB3);
	PORTB &= ~(1 << PB3);
}

void holdBCMSideButtonLine()
{
	PORTB |= (1 << PB3);
}

void releaseBCMSideButtonLine()
{
	PORTB &= ~(1 << PB3);
}

void initBCMSideButtonPulseController()
{
	pulseButtonState = BUTTON_IDLE;
	pulseRequests = 0;
	abortOngoingPulseRequestFlag = 0;
}

void runBCMSideButtonPulseController()
{
	if (checkAbortOngoingPulseRequestFlag())
	{
		pulseRequests = 0;
		releaseBCMSideButtonLine();
		pulseButtonState = BUTTON_IDLE;
		return;
	}
	switch (pulseButtonState)
	{
		case BUTTON_IDLE:
		if (pulseRequests > 0) // Low enabled, button pressed
		{
			--pulseRequests;
			pulseButtonState = BUTTON_PRESSED;
			pulseDurationEndingTime = readTimerMillis() + PULSE_DURATION_TIME_MILLIS;
			holdBCMSideButtonLine();
			break;
		}
		else
		{
			pulseButtonState = BUTTON_IDLE;
			break;
		}
		break;
		case BUTTON_PRESSED:
		if (checkDelayUntil(pulseDurationEndingTime))
		{
			pulseButtonState = BUTTON_DEPRESSED;
			pulseSettleEndingTime = readTimerMillis() + PULSE_SETTLING_TIME_MILLIS;
			releaseBCMSideButtonLine();
			break;
		}
		else
		{
			pulseButtonState = BUTTON_PRESSED;
			holdBCMSideButtonLine();
			break;
		}
		break;
		case BUTTON_DEPRESSED:
		if (checkDelayUntil(pulseSettleEndingTime))
		{
			pulseButtonState = BUTTON_IDLE;
			releaseBCMSideButtonLine();
			break;
		}
		else
		{
			pulseButtonState = BUTTON_DEPRESSED;
			releaseBCMSideButtonLine();
			break;
		}
		break;
		default:
		pulseButtonState = BUTTON_DEPRESSED;
		pulseSettleEndingTime = readTimerMillis() + PULSE_SETTLING_TIME_MILLIS;
		releaseBCMSideButtonLine();
		break;
	}
}

void queuePulseForBCMSideButtonLine()
{
	if (pulseRequests < 255)
	{
		++pulseRequests;
	}
}

void abortBCMSideButtonOngoingPulse()
{
	abortOngoingPulseRequestFlag = 1;
}

uint8_t checkAbortOngoingPulseRequestFlag()
{
	if (abortOngoingPulseRequestFlag)
	{
		abortOngoingPulseRequestFlag = 0;
		return 1;
	}
	return 0;
}

uint8_t isBCMSideButtonPulseOngoing()
{
	if (pulseButtonState == PULSE_IDLE)
	{
		return PULSE_IDLE;
	}
	else
	{
		return PULSE_ONGOING;
	}
}

void forwardPhysicalButtonStatusToBCMSideButtonLine()
{
	if (!readPhysicalButtonRaw())
	{
		holdBCMSideButtonLine();
	}
	else
	{
		releaseBCMSideButtonLine();
	}
}