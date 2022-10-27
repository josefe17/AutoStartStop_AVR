/*
 * AutoStartStop_AVR.c
 *
 * Created: 27/10/2022 18:25:51
 * Author : josefe
 */ 

#include <avr/io.h>

#define BUTTON_NO_PRESS 0
#define BUTTON_SHORT_PRESS 1
#define BUTTON_LONG_PRESS 2

enum ButtonFSMStates
{
	BUTTON_IDLE,
	BUTTON_PRESSED,
	BUTTON_RELEASE_PENDING
};

// Check if auto start stop is currently enabled or disabled by feedback LED signal
// to avoid the device pulsing continuously, ensure this board is powered only while
// the feedback LED is operative
uint8_t readAutoStartStopCurrentStatus();
// Pulse the start stop button signal
//TODO
void pulseButton();
// Hold the start stop button signal pressed
void holdButton();
// set the start stop button signal depressed
void releaseButton();
// Turn feedback LED output on
void turnLEDOn();
// Turn feedback LED output off
void turnLEDOff();
// Configure the input button FSM variables
void initUserButton();
// Run the user side button FSM machine
uint8_t processUserButton();
// Read the user side button line
uint8_t readButtonRaw();

enum ButtonFSMStates userButtonState;

int main(void)
{
	volatile uint8_t autoStarStopExpectedStatus;
	volatile uint8_t switchOverrideMode;
		
	// Init GPIO	
	// TODO
	// ENABLE PULLUPS
	PORTB = (1 << PB1) | (1 << PB2);
	DDRB = (1 << PB4) | (1 << PB1);
	// Read EEPROM
	// TODO
	// switchOverrrideMode and autoStarStopExpectedStatus	
	void initUserButton(); // FSM init
    while (1) 
    {
		volatile uint8_t userButtonStatus = (volatile uint8_t) processUserButton();			
		
		switch (userButtonStatus)
		{
			case BUTTON_LONG_PRESS:		
				if (switchOverrideMode)
				{
					switchOverrideMode = 0;
				}
				else
				{
					switchOverrideMode = 1;
				}
				// TODO
				// Store switchOverrrideMode in EEPROM
			break;	
			case BUTTON_SHORT_PRESS:
		//if (userButtonStatus == BUTTON_SHORT_PRESS && !switchOverrrideMode)		
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
					// TODO
					// Store autoStarStopExpectedStatus in EEPROM
				break;
				}
			// If override mode continues with next case			
			case BUTTON_NO_PRESS:
			default:
				if (switchOverrideMode)
				{
					if (!readButtonRaw())
					{
						holdButton();				
					}
					else
					{
						releaseButton();
					}
					if (readAutoStartStopCurrentStatus())
					{
						turnLEDOff();
					}
					else
					{
						turnLEDOn();
					}
				}
				else
				{	
					if ((!readAutoStartStopCurrentStatus() && autoStarStopExpectedStatus) //ASS off but required on
						|| (readAutoStartStopCurrentStatus() && !autoStarStopExpectedStatus)) // ASS on but required off
					{
						pulseButton();			
					}
					if (readAutoStartStopCurrentStatus())
					{
						turnLEDOff();
					}
					else
					{
						turnLEDOn();
					}
			 
				}
			break;
		}
    }
}

uint8_t readAutoStartStopCurrentStatus() // 0 for LED on so ASS off
{
	if (PINB & (1 << PB3))
	{
		return 0;
	} 
	return 1;
}

void pulseButton()
{
	//TODO
}

void holdButton()
{
	// TODO
	//Check wether to stop pulse timer here
	PORTB |= (1 << PB4);
}

void releaseButton()
{
	PORTB &= ~(1 << PB4);
}

void turnLEDOn()
{
	PORTB &= ~(1 << PB1);
}

void turnLEDOff()
{
	PORTB |= (1 << PB1);
}

void initUserButton()
{
	userButtonState = ButtonFSMStates.BUTTON_IDLE;
}

uint8_t processUserButton()
{
	switch (userButtonState)
	{
		case ButtonFSMStates.BUTTON_IDLE:
			if (!readButtonRaw()) // Low enabled, button pressed
			{
				userButtonState = ButtonFSMStates.BUTTON_PRESSED;
				//TODO
				//Start timer
				return BUTTON_NO_PRESS;
			}
			else
			{
				userButtonState = ButtonFSMStates.BUTTON_IDLE;
				//TODO
				//Stop timer
				return BUTTON_NO_PRESS;
			}
		break;
		case ButtonFSMStates.BUTTON_PRESSED:
			if (0) // TODO check timer overflow true
			{
				userButtonState = ButtonFSMStates.BUTTON_RELEASE_PENDING;
				// TODO
				//Stop timer
				return BUTTON_LONG_PRESS;				
			}
			else
			{
				if (!readButtonRaw()) // Low enabled, button pressed
				{
					userButtonState = ButtonFSMStates.BUTTON_PRESSED;
					return BUTTON_NO_PRESS;
				}
				else
				{
					userButtonState = ButtonFSMStates.BUTTON_IDLE;
					//TODO
					//Stop timer
					return BUTTON_SHORT_PRESS;
				}
			}
		break;
		case ButtonFSMStates.BUTTON_RELEASE_PENDING:
			if (!readButtonRaw()) // Low enabled, button pressed
			{
				userButtonState = ButtonFSMStates.BUTTON_RELEASE_PENDING;
				// TODO
				//Stop timer
				return BUTTON_NO_PRESS;
			}
			else
			{
				userButtonState = ButtonFSMStates.BUTTON_IDLE;
				//TODO
				//Stop timer
				return BUTTON_NO_PRESS;
			}
		 break;
		 default:
			return BUTTON_NO_PRESS;
	}
}

uint8_t readButtonRaw()
{
	return (PINB & (1 << PB1)); // Low enabled, button pressed
}
