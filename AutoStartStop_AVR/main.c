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

/*GPIO functions*/
/* User buttom FSM functions*/
// Configure the input button FSM variables
void initUserButton();
// Run the user side button FSM machine
void processUserButton();
// Output function
void pulseButton();

// Input button FSM state
enum ButtonFSMStates userButtonState;

// Stores the ASS consigned value that must coincide with the system one
volatile uint8_t autoStarStopExpectedStatus;
// Stores if the switch operating mode has changed
volatile uint8_t switchOverrideMode;

// Time instant when a button pressure is considered valid short or long press
uint16_t buttonPressMinimumThresholdTime;

int main(void)
{
		
	// Init GPIO	
	initPhysicalButton();
	initBCMSideButtonLine();
	initPhysicalLED();	
	initBCMSideLED();
	releaseBCMSideButtonLine();
	turnPhysicalLEDOff();
	// initialize adn read EEPROM
	initEEPROM();
	readEEPROM((uint8_t*) &autoStarStopExpectedStatus, (uint8_t*) &switchOverrideMode);		
	// Tick timer init		
	initTimerMillis();
	sei();	
	initUserButton(); // FSM init
	initBCMSideButtonPulseController();
    while (1) 
    {
		// Read button
		processUserButton();			
		
		// If the system is in override mode
		if (switchOverrideMode)
		{
			// Button control is forwarded
			forwardPhysicalButtonStatusToBCMSideButtonLine();
		}
		// Else handle the pulse
		else
		{
			pulseButton();
		}
		
		// Forward the LED
		forwardBCMSideLEDStatusToPhysicalLED();
		// Process the button pulses
		runBCMSideButtonPulseController();
		// EEPROM is only written if is marked for update and data have changed
		processEEPROM(autoStarStopExpectedStatus,switchOverrideMode);
    }
}

void pulseButton()
{
	// If ASS feedback and expected status mismatch and not ongoing pulse, pulse the line
	if (((!readAutoStartStopCurrentStatus() && autoStarStopExpectedStatus) //ASS off but required on
	|| (readAutoStartStopCurrentStatus() && !autoStarStopExpectedStatus)) // ASS on but required off
	&& !isBCMSideButtonPulseOngoing()) // No ongoing pulse
	{
		queuePulseForBCMSideButtonLine();
		// TODO
		// Think about holding the line pressed until match
	}	
}

void initUserButton()
{
	userButtonState = BUTTON_IDLE;	
}

void processUserButton()
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
				userButtonState = BUTTON_RELEASE_PENDING;					
				if (switchOverrideMode)
				{					
					switchOverrideMode = 0;
					// make 1 flash to indicate memory mode on
				}
				else
				{
					switchOverrideMode = 1;
					// make 3 fast fading flashes to indicate memory mode off
				}
				setEEPROMDirtyFlag();					
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
					break;
				}
				else
				{
					userButtonState = BUTTON_RELEASE_DEBOUNCE;
					break;
				}
			}
			break;
		case BUTTON_RELEASE_PENDING:
			if (!readPhysicalButtonRaw()) // Low enabled, button pressed
			{
				userButtonState = BUTTON_RELEASE_PENDING;
				break;
			}
			else
			{
				userButtonState = BUTTON_RELEASE_PENDING_DEBOUNCE;
				buttonPressMinimumThresholdTime = readTimerMillis() + BUTTON_DEBOUNCE_DURATION_MILLIS;
				break;
			}
			break;
		case BUTTON_RELEASE_PENDING_DEBOUNCE:
			if (!readPhysicalButtonRaw()) // Low enabled, button pressed
			{
				userButtonState = BUTTON_RELEASE_PENDING;				
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
					userButtonState = BUTTON_RELEASE_PENDING_DEBOUNCE;
					break;
				}
			}
			break;			
		default:
			break;
	}		
}