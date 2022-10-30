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

#define PULSE_ONGOING 1
#define PULSE_IDLE 0

#define EEPROM_ADDRESS 0

#define BUTTON_FSM_DELAY_MILLIS 50
#define PULSE_FSM_DELAY_MILLIS 1

#define BUTTON_LONG_PRESS_DURATION_MILLIS 4000

#define PULSE_DURATION_TIME_MILLIS 250
#define PULSE_SETTLING_TIME_MILLIS 200

enum ButtonFSMStates
{
	BUTTON_IDLE,
	BUTTON_PRESSED,
	BUTTON_RELEASE_PENDING,
	BUTTON_DEPRESSED
};



/*GPIO functions*/
// Check if auto start stop is currently enabled or disabled by feedback LED signal
// to avoid the device pulsing continuously, ensure this board is powered only while
// the feedback LED is operative
uint8_t readAutoStartStopCurrentStatus();
// Read the user side button line
uint8_t readButtonRaw();
// Hold the start stop button signal pressed
void holdButton();
// set the start stop button signal depressed
void releaseButton();
// Turn feedback LED output on
void turnLEDOn();
// Turn feedback LED output off
void turnLEDOff();

/* Pulse FSM functions */
// Init FSM
void initPulse();
// Process FSM
void processPulse();
// Input function
void pulseButton();
// Output function
uint8_t isPulseOngoing();

/* User buttom FSM functions*/
// Configure the input button FSM variables
void initUserButton();
// Run the user side button FSM machine
uint8_t processUserButton();

/*EEPROM functions*/
// Read the EEPROM contains and update the flags
void readEEPROM(uint8_t* autoStarStopExpectedStatusPointer, uint8_t* switchOverrideModePointer);
// Write the flag contents to the EEPROM
void writeEEPROM(uint8_t autoStarStopExpectedStatusValue, uint8_t switchOverrideModeValue);
// Compute the number of 1's in a byte
uint8_t countOnes(uint8_t number);

/* Timer 1 functions */
// Configures the Timer 1 for millis operation
void initTimerMillis();
// Read the Timer 1 for millis operation
uint16_t readTimerMillis();
// Check if that time instant is elapsed or not
uint8_t checkDelayUntil(uint16_t instant);

// Input button FSM state
enum ButtonFSMStates userButtonState;
// Pulse output FSM state
enum ButtonFSMStates pulseButtonState;
// Time instant when to fire the button FSM again
uint16_t buttonFSMEndOfDelayTime;
// Time instant when to fire the pulse FSM again
uint16_t pulseFSMEndOfDelayTime;
// Time instant when the simulated button depression ends
uint16_t pulseDurationEndingTime;
// Time instant when the simulated button settling time ends
uint16_t pulseSettleEndingTime;
// Time instant when a button pressure is considered a long press
uint16_t buttonLongPressMinimumThresholdTime;
// Counter of the number of pending pulse requests
uint8_t pulseRequests;

// Timer postscaler / counter
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
	initTimerMillis();
	sei();
	initUserButton(); // FSM init
	initPulse();
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
			// If the button wasn't pressed		
			// TODO
			// move to pulse FSM
			case BUTTON_NO_PRESS:
			default:
				// If the system is in override mode
				if (switchOverrideMode)
				{
					// Pass the button and LED actions
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
				// Else handle the pulse
				else
				{	// If ASS feedback and expected status mismatch and not ongoing pulse, pulse the line
					if (((!readAutoStartStopCurrentStatus() && autoStarStopExpectedStatus) //ASS off but required on
						|| (readAutoStartStopCurrentStatus() && !autoStarStopExpectedStatus)) // ASS on but required off
						&& !isPulseOngoing()) // No ongoing pulse
					{
						pulseButton();			
					}
					// Forward the LED
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
		// Process the button pulses
		// TODO check disabling mode
		processPulse();
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
	++pulseRequests;
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

void initPulse()
{
	pulseButtonState = BUTTON_IDLE;
	pulseFSMEndOfDelayTime = 0;
	pulseRequests = 0;
}

void processPulse()
{
	if (checkDelayUntil(pulseFSMEndOfDelayTime))
	{
		pulseFSMEndOfDelayTime = readTimerMillis() + PULSE_FSM_DELAY_MILLIS;
		switch (pulseButtonState)
		{
			case BUTTON_IDLE:
				if (pulseRequests > 0) // Low enabled, button pressed
				{
					--pulseRequests;
					pulseButtonState = BUTTON_PRESSED;
					pulseDurationEndingTime = readTimerMillis() + PULSE_DURATION_TIME_MILLIS;					
					holdButton();			
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
					releaseButton();					
					break;
				}
				else
				{
					pulseButtonState = BUTTON_PRESSED;
					holdButton();								
					break;
				}
				break;
			case BUTTON_DEPRESSED:
				if (checkDelayUntil(pulseSettleEndingTime))
				{
					pulseButtonState = BUTTON_IDLE;
					releaseButton();					
					break;
				}
				else
				{
					pulseButtonState = BUTTON_DEPRESSED;
					releaseButton();
					break;
				}
				break;
			default:
				pulseButtonState = BUTTON_DEPRESSED;
				pulseSettleEndingTime = readTimerMillis() + PULSE_SETTLING_TIME_MILLIS;
				releaseButton();
				break;			
		}
	}
}

uint8_t isPulseOngoing()
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

void initUserButton()
{
	userButtonState = BUTTON_IDLE;
	buttonFSMEndOfDelayTime = 0;
}

uint8_t processUserButton()
{
	uint8_t returnValue = BUTTON_NO_PRESS;
	if (checkDelayUntil(buttonFSMEndOfDelayTime))
	{
		buttonFSMEndOfDelayTime = readTimerMillis() + BUTTON_FSM_DELAY_MILLIS;
		switch (userButtonState)
		{
			case BUTTON_IDLE:
				if (!readButtonRaw()) // Low enabled, button pressed
				{
					userButtonState = BUTTON_PRESSED;
					buttonLongPressMinimumThresholdTime = readTimerMillis() + BUTTON_LONG_PRESS_DURATION_MILLIS;
					returnValue = BUTTON_NO_PRESS;
					break;
				}
				else
				{
					userButtonState = BUTTON_IDLE;
					returnValue = BUTTON_NO_PRESS;
					break;
				}
			break;
			case BUTTON_PRESSED:
				if (checkDelayUntil(buttonLongPressMinimumThresholdTime))
				{
					userButtonState = BUTTON_RELEASE_PENDING;
					returnValue = BUTTON_LONG_PRESS;
					break;
				}
				else
				{
					if (!readButtonRaw()) // Low enabled, button pressed
					{
						userButtonState = BUTTON_PRESSED;
						returnValue = BUTTON_NO_PRESS;
						break;
					}
					else
					{
						userButtonState = BUTTON_IDLE;
						returnValue = BUTTON_SHORT_PRESS;
						break;
					}
				}
			break;
			case BUTTON_RELEASE_PENDING:
				if (!readButtonRaw())
				{
					userButtonState = BUTTON_RELEASE_PENDING;
					returnValue = BUTTON_NO_PRESS;
					break;
				}
				else
				{
					userButtonState = BUTTON_IDLE;
					returnValue = BUTTON_NO_PRESS;
					break;
				}
			 break;
			 default:
				returnValue = BUTTON_NO_PRESS;
				break;
		}		
	}
	return returnValue;
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

uint16_t readTimerMillis()
{
	uint16_t aux;
	TIMSK |= ~(1 << OCIE1A); // Disable timer interrupts
	aux = millisCount;
	TIMSK |= (1 << OCIE1A); // Enable timer interrupts
	return aux;
}

uint8_t checkDelayUntil(uint16_t instant)
{
	return (uint8_t) (readTimerMillis() >= instant);
}

ISR(TIMER1_COMPA_vect)
{
	++millisCount;
}
