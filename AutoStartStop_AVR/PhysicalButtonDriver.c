/*
 * PhysicalButtonDriver.c
 *
 * Created: 02/11/2023 20:23:44
 *  Author: josefe
 */ 


#include "PhysicalButtonDriver.h"
#include "TimerMillis.h"
#include "ButtonVariables.h"

// Input button FSM state
enum ButtonFSMStates userButtonState;

// Short press flag
uint8_t shortPressFlag;

// Long press flag
uint8_t longPressFlag;

// Lon press release pending flag
uint8_t lonPressReleasePendingFlag;

// Time instant when a button pressure is considered valid short or long press
uint16_t buttonPressMinimumThresholdTime;

// PB2 physical button input, with DDR = 0 (expected default value) and PORT = 1 (input pullup)
void initPhysicalButtonLine()
{
	MCUCR &= ~(1 << PUD);
	DDRB &= ~(1 << PB2);
	PORTB |= (1 << PB2);
}

uint8_t readPhysicalButtonRaw()
{
	return (PINB & (1 << PB2)); // Low enabled, button pressed
}

uint8_t checkPhysicalButtonShortPress()
{
	if (shortPressFlag)
	{
		shortPressFlag = 0;
		return 1;
	}
	return 0;
}

uint8_t checkPhysicalButtonLongPress()
{
	if (longPressFlag)
	{
		longPressFlag = 0;
		return 1;
	}
	return 0;
}

uint8_t checkPhysicalButtonLongPressReleasePending()
{
	if (lonPressReleasePendingFlag)
	{
		lonPressReleasePendingFlag = 0;
		return 1;
	}
	return 0;
}

void initPhysicalButtonController()
{
	userButtonState = BUTTON_IDLE;
	shortPressFlag = 0;
	longPressFlag = 0;
	lonPressReleasePendingFlag = 0;
}

void runPhysicalButtonController()
{
	switch (userButtonState)
	{
		case BUTTON_IDLE:
		if (!readPhysicalButtonRaw()) // Low enabled, button pressed
		{
			userButtonState = BUTTON_PRESSED_DEBOUNCE;
			buttonPressMinimumThresholdTime = readTimerMillis() + BUTTON_DEBOUNCE_DURATION_MILLIS;
			break;
		}
		else // No button
		{
			userButtonState = BUTTON_IDLE;
			break;
		}
		break;
		case BUTTON_PRESSED_DEBOUNCE:
		if (readPhysicalButtonRaw()) // Button released, so unvalid, back to idle
		{
			userButtonState = BUTTON_IDLE;
			break;
		}
		else
		{
			if (checkDelayUntil(buttonPressMinimumThresholdTime)) // Timeout
			{
				userButtonState = BUTTON_PRESSED;
				buttonPressMinimumThresholdTime = readTimerMillis() + BUTTON_LONG_PRESS_DURATION_MILLIS - BUTTON_DEBOUNCE_DURATION_MILLIS;
				break;
			}
			else // No timeout
			{
				userButtonState = BUTTON_PRESSED_DEBOUNCE;
				break;
			}
			break;
		}
		case BUTTON_PRESSED:
		if (checkDelayUntil(buttonPressMinimumThresholdTime))
		{
			userButtonState = BUTTON_LONG_PRESS_RELEASE_PENDING;
			longPressFlag = 1;
			lonPressReleasePendingFlag = 1;
			break;
		}
		else
		{
			if (!readPhysicalButtonRaw()) // Low enabled, button pressed
			{
				userButtonState = BUTTON_PRESSED;
				break;
			}
			else
			{
				userButtonState = BUTTON_RELEASE_DEBOUNCE;
				buttonPressMinimumThresholdTime = readTimerMillis() + BUTTON_DEBOUNCE_DURATION_MILLIS;
				break;
			}
			break;
		}
		break;
		case BUTTON_RELEASE_DEBOUNCE:
		if (!readPhysicalButtonRaw()) // Low enabled, button pressed
		{
			userButtonState = BUTTON_PRESSED;
			buttonPressMinimumThresholdTime = readTimerMillis() + BUTTON_LONG_PRESS_DURATION_MILLIS - BUTTON_DEBOUNCE_DURATION_MILLIS;
			break;
		}
		else
		{
			if (checkDelayUntil(buttonPressMinimumThresholdTime))
			{
				
				userButtonState = BUTTON_IDLE;
				shortPressFlag = 1;
				break;
			}
			else
			{
				userButtonState = BUTTON_RELEASE_DEBOUNCE;
				break;
			}
		}
		break;
		case BUTTON_LONG_PRESS_RELEASE_PENDING:
		if (!readPhysicalButtonRaw()) // Low enabled, button pressed
		{
			userButtonState = BUTTON_LONG_PRESS_RELEASE_PENDING;
			lonPressReleasePendingFlag = 1;
			break;
		}
		else
		{
			userButtonState = BUTTON_LONG_PRESS_RELEASE_PENDING_DEBOUNCE;
			buttonPressMinimumThresholdTime = readTimerMillis() + BUTTON_DEBOUNCE_DURATION_MILLIS;
			lonPressReleasePendingFlag = 1;
			break;
		}
		break;
		case BUTTON_LONG_PRESS_RELEASE_PENDING_DEBOUNCE:
		if (!readPhysicalButtonRaw()) // Low enabled, button pressed
		{
			userButtonState = BUTTON_LONG_PRESS_RELEASE_PENDING;
			lonPressReleasePendingFlag = 1;
			break;
		}
		else
		{
			if (checkDelayUntil(buttonPressMinimumThresholdTime))
			{
				userButtonState = BUTTON_IDLE;
				break;
			}
			else
			{
				userButtonState = BUTTON_LONG_PRESS_RELEASE_PENDING_DEBOUNCE;
				lonPressReleasePendingFlag = 1;
				break;
			}
		}
		break;
		default:
		break;
	}
}