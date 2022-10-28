/*
 * AutoStartStop_AVR.c
 *
 * Created: 27/10/2022 18:25:51
 * Author : josefe
 */ 

#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#define BUTTON_NO_PRESS 0
#define BUTTON_SHORT_PRESS 1
#define BUTTON_LONG_PRESS 2

#define EEPROM_ADDRESS 0

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
// Read the EEPROM contains and update the flags
void readEEPROM(uint8_t* autoStarStopExpectedStatusPointer, uint8_t* switchOverrideModePointer);
// Write the flag contents to the EEPROM
void writeEEPROM(uint8_t autoStarStopExpectedStatusValue, uint8_t switchOverrideModeValue);
// Compute the number of 1's in a byte
uint8_t countOnes(uint8_t number);
// Configures the Timer 1 for system tick millis operation
void initTimerMillis();

enum ButtonFSMStates userButtonState;
volatile uint16_t millisCount;

int main(void)
{
	volatile uint8_t autoStarStopExpectedStatus;
	volatile uint8_t switchOverrideMode;
	volatile uint8_t userButtonStatus;
	volatile uint8_t markEEPROMForUpdate;
		
	// Init GPIO	
	// TODO
	// ENABLE PULLUPS
	PORTB = (1 << PB1) | (1 << PB2);
	DDRB = (1 << PB4) | (1 << PB1);
	releaseButton();
	turnLEDOff();
	// Read EEPROM
	readEEPROM((uint8_t*) &autoStarStopExpectedStatus, (uint8_t*) &switchOverrideMode);
	markEEPROMForUpdate = 0;	
	// Tick timer init
	millisCount = 0;
	sei();
	void initUserButton(); // FSM init
    while (1) 
    {
		userButtonStatus = (volatile uint8_t) processUserButton();			
		
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
				markEEPROMForUpdate = 1;
			break;	
			case BUTTON_SHORT_PRESS:				
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
					markEEPROMForUpdate = 1;
					break;
				}
			// If in override mode, continues with next case			
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
		// EEPROM is only written if is marked for update and data have changed
		if (markEEPROMForUpdate && (eeprom_is_ready()))
		{
			writeEEPROM(autoStarStopExpectedStatus,switchOverrideMode);
			markEEPROMForUpdate = 0;			
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
	userButtonState = BUTTON_IDLE;
}

uint8_t processUserButton()
{
	switch (userButtonState)
	{
		case BUTTON_IDLE:
			if (!readButtonRaw()) // Low enabled, button pressed
			{
				userButtonState = BUTTON_PRESSED;
				//TODO
				//Start timer
				return BUTTON_NO_PRESS;
			}
			else
			{
				userButtonState = BUTTON_IDLE;
				//TODO
				//Stop timer
				return BUTTON_NO_PRESS;
			}
		break;
		case BUTTON_PRESSED:
			if (0) // TODO check timer overflow true
			{
				userButtonState = BUTTON_RELEASE_PENDING;
				// TODO
				//Stop timer
				return BUTTON_LONG_PRESS;				
			}
			else
			{
				if (!readButtonRaw()) // Low enabled, button pressed
				{
					userButtonState = BUTTON_PRESSED;
					return BUTTON_NO_PRESS;
				}
				else
				{
					userButtonState = BUTTON_IDLE;
					//TODO
					//Stop timer
					return BUTTON_SHORT_PRESS;
				}
			}
		break;
		case BUTTON_RELEASE_PENDING:
			if (!readButtonRaw()) // Low enabled, button pressed
			{
				userButtonState = BUTTON_RELEASE_PENDING;
				// TODO
				//Stop timer
				return BUTTON_NO_PRESS;
			}
			else
			{
				userButtonState = BUTTON_IDLE;
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


void readEEPROM(uint8_t* autoStarStopExpectedStatusPointer, uint8_t* switchOverrideModePointer)
{
	uint8_t value = eeprom_read_byte((uint8_t*) EEPROM_ADDRESS);
	uint8_t lowerNibble = value & 0x0F;
	uint8_t upperNibble = (value >> 4) & 0x0F;	
	if (countOnes(lowerNibble) > 2)
	{
		(*autoStarStopExpectedStatusPointer) = 1;
	}
	else
	{
		(*autoStarStopExpectedStatusPointer) = 0;
	}
	if (countOnes(upperNibble) > 2)
	{
		(*switchOverrideModePointer) = 1;
	}
	else
	{
		(*switchOverrideModePointer) = 0;
	}
}

void writeEEPROM(uint8_t autoStarStopExpectedStatusValue, uint8_t switchOverrideModeValue)
{
	uint8_t value = 0;
	if (autoStarStopExpectedStatusValue)
	{
		value |= 0x0F;
	}
	if (switchOverrideModeValue)
	{
		value |= 0xF0;
	}
	eeprom_update_byte((uint8_t*) EEPROM_ADDRESS, value);
}

uint8_t countOnes(uint8_t number)
{
	uint8_t count = 0;
	for (uint8_t index = 0; index < 7; ++index)
	{
		if (number & (1 << index))
		{
			++count;
		}
	}
	return count;
}

void initTimerMillis()
{
	GTCCR = (1 << PSR1); // Clear prescaler
	TCNT1 = 0; // Clear counter
	OCR1A = 125; // Set count
	TIFR = (1 << OCF1A); // Clear IF
	TIMSK = (1 << OCIE1A); // Enable interrupts
	TCCR1 = (1 << CTC1) | 3; // /64 prescaler and CTC	
}

ISR(TIMER1_COMPA_vect)
{
	++millisCount;
}
