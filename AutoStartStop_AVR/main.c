/*
 * AutoStartStop_AVR.c
 *
 * Created: 27/10/2022 18:25:51
 * Author : josefe
 */ 

#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "ButtonVariables.h"
#include "TimerMillis.h"
#include "EEPROMController.h"
#include "PhysicalButtonDriver.h"
#include "PhysicalLEDDriver.h"
#include "BCMSideButtonDriver.h"
#include "BCMSideLEDDriver.h"
#include "DebugGPIO.h"

void handleShortPress();
void handleLongPress();
void updateStartStopStatus();
void updateFeedbackLEDStatus();

// Stores the ASS consigned value that must coincide with the system one
volatile uint8_t autoStarStopExpectedStatus;
// Stores if the switch operating mode has changed
volatile uint8_t switchOverrideMode;


int main(void)
{
		
	// Initialize GPIO	
	initPhysicalButtonLine();
	initBCMSideButtonLine();
	initPhysicalLED();	
	initBCMSideLED();
	initDebugGPIOInputPullup();
	// Set initial values for outputs
	releaseBCMSideButtonLine();
	turnPhysicalLEDOff();
	// Initialize and read EEPROM
	initEEPROM();
	readEEPROM((uint8_t*) &autoStarStopExpectedStatus, (uint8_t*) &switchOverrideMode);		
	// Initialize tick timer and interrupts	
	initTimerMillis();
	sei();	
	// Initialize software
	initPhysicalButtonController();
	initBCMSideButtonPulseController();
	initPhysicalLEDBlinkController();
	initBCMSideLEDFilter();	
	uint16_t startUpDelayTimer = readTimerMillis() + STARTUP_DELAY_MILLIS;
	while (checkDelayUntil(startUpDelayTimer))
	{
	}
    while (1) 
    {
		// Read and process physical button
		runPhysicalButtonController();	
		// Read and process BCM side LED line (feedback)
		runBCMSideLEDFilter();		
		// Check processed button inputs
		handleLongPress();
		handleShortPress();
		// Control the start stop status according to user requests
		updateStartStopStatus();
		// Control the button LED according to user settings
		updateFeedbackLEDStatus();
		// Process physical LED blinks
		runPhysicalLEDBlinks();
		// Process the button pulses
		runBCMSideButtonPulseController();
		// EEPROM is only written if is marked for update and data have changed
		// Also, LED is blinked once if debug pin (PB0) is shorted to ground and a writing was performed
		if (processEEPROM(autoStarStopExpectedStatus,switchOverrideMode) && !readDebugGPIO())
		{
			requestPhysicalLEDShortBlink();
		}
    }
}

void handleShortPress()
{
	if (checkPhysicalButtonShortPress())
	{
		if (!switchOverrideMode)
		{
			if (autoStarStopExpectedStatus)
			{
				autoStarStopExpectedStatus = 0;
			}
			else
			{
				autoStarStopExpectedStatus = 1;
			}
			setEEPROMDirtyFlag();
		}
	}
}

void handleLongPress()
{
	if (checkPhysicalButtonLongPress())
	{
		if (switchOverrideMode)
		{
			// Memory mode
			switchOverrideMode = 0;
			requestPhysicalLEDThreeBlinks();
		}
		else
		{
			// Bypass mode (factory operation)
			switchOverrideMode = 1;
			requestPhysicalLEDLongBlink();			
		}
		setEEPROMDirtyFlag();
	}
}

void updateStartStopStatus()
{
	// If a release is pending, disable any further actions and release the line
	if (checkPhysicalButtonLongPressReleasePending())
	{
		releaseBCMSideButtonLine();
	}
	else
	{
		// If the system is in override mode
		if (switchOverrideMode)
		{
			// Button control is forwarded
			forwardPhysicalButtonStatusToBCMSideButtonLine();
		}
		// Else handle the pulse
		else
		{
			if ((!readBCMSideLEDLineFiltered() && autoStarStopExpectedStatus) //ASS off but required on
			|| (readBCMSideLEDLineFiltered() && !autoStarStopExpectedStatus)) // ASS on but required off
			{
				holdBCMSideButtonLine(); // Press the button until they match
			}
			else
			{
				releaseBCMSideButtonLine();
			}
		}
	}	
}

void updateFeedbackLEDStatus()
{
	// Forward the LED only if no blinking is running
	if (!isPhysicalLEDBlinkingSequenceRunning())
	{
		forwardBCMSideLEDStatusToPhysicalLED();
	}
}