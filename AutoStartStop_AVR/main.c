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

#define BUTTON_DEBOUNCE_DURATION_MILLIS 50
#define BUTTON_LONG_PRESS_DURATION_MILLIS 4000

#define PULSE_DURATION_TIME_MILLIS 250
#define PULSE_SETTLING_TIME_MILLIS 200

enum ButtonFSMStates
{
	BUTTON_IDLE,
	BUTTON_PRESSED,
	BUTTON_PRESSED_DEBOUNCE,
	BUTTON_RELEASE_PENDING,
	BUTTON_RELEASE_DEBOUNCE,
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
// Set the button signal according to the user button
void forwardButton();
// Turn feedback LED output on
void turnLEDOn();
// Turn feedback LED output off
void turnLEDOff();
// Controls the LED according to the feedback line status
void forwardLED();

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
void processUserButton();

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

// Stores the ASS consigned value that must coincide with the system one
volatile uint8_t autoStarStopExpectedStatus;
// Stores if the switch operating mode has changed
volatile uint8_t switchOverrideMode;
// Set if there were any changes to the NVM contents and writing is required
volatile uint8_t markEEPROMForUpdate;
// Counter of the number of pending pulse requests
volatile uint8_t pulseRequests;

// Time instant when the simulated button depression ends
uint16_t pulseDurationEndingTime;
// Time instant when the simulated button settling time ends
uint16_t pulseSettleEndingTime;
// Time instant when a button pressure is considered valid short or long press
uint16_t buttonPressMinimumThresholdTime;

// Timer postscaler / counter
volatile uint16_t millisCount;

int main(void)
{
		
	// Init GPIO	
	MCUCR &= ~(1 << PUD);
	PORTB = (1 << PB1) | (1 << PB2);
	DDRB = (1 << PB3) | (1 << PB1);
	releaseButton();
	turnLEDOff();
	// Read EEPROM
	readEEPROM((uint8_t*) &autoStarStopExpectedStatus, (uint8_t*) &switchOverrideMode);		
	// Tick timer init
	millisCount = 0;	
	initTimerMillis();
	sei();
	initUserButton(); // FSM init
	initPulse();
    while (1) 
    {
		// Read button
		processUserButton();			
		
		// If the system is in override mode
		if (switchOverrideMode)
		{
			// Button control is forwarded
			forwardButton();
		}
		// Else handle the pulse
		else
		{
			pulseButton();
		}
		// Forward the LED
		forwardLED();
		// Process the button pulses
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
	if (PINB & (1 << PB4))
	{
		return 0;
	} 
	return 1;
}

void pulseButton()
{
	// If ASS feedback and expected status mismatch and not ongoing pulse, pulse the line
	if (((!readAutoStartStopCurrentStatus() && autoStarStopExpectedStatus) //ASS off but required on
	|| (readAutoStartStopCurrentStatus() && !autoStarStopExpectedStatus)) // ASS on but required off
	&& !isPulseOngoing()) // No ongoing pulse
	{
		++pulseRequests;
	}	
}

void holdButton()
{
	PORTB |= (1 << PB3);
}

void releaseButton()
{
	PORTB &= ~(1 << PB3);
}

void forwardButton()
{
	if (!readButtonRaw())
	{
		holdButton();
	}
	else
	{
		releaseButton();
	}
}

void turnLEDOn()
{
	PORTB &= ~(1 << PB1);
}

void turnLEDOff()
{
	PORTB |= (1 << PB1);
}

void forwardLED()
{
	if (readAutoStartStopCurrentStatus())
	{
		turnLEDOff();
	}
	else
	{
		turnLEDOn();
	}
}

void initPulse()
{
	pulseButtonState = BUTTON_IDLE;	
	pulseRequests = 0;
}

void processPulse()
{
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
	markEEPROMForUpdate = 0;
}

void processUserButton()
{
	switch (userButtonState)
	{
		case BUTTON_IDLE:
			if (!readButtonRaw()) // Low enabled, button pressed
			{
				userButtonState = BUTTON_PRESSED_DEBOUNCE;
				buttonPressMinimumThresholdTime = readTimerMillis() + BUTTON_DEBOUNCE_DURATION_MILLIS;
				break;
			}
			else
			{
				userButtonState = BUTTON_IDLE;
				break;
			}
		break;
		case BUTTON_PRESSED_DEBOUNCE:
			if (checkDelayUntil(buttonPressMinimumThresholdTime))
			{
				if (!readButtonRaw()) // Low enabled, button pressed
				{
					userButtonState = BUTTON_PRESSED;
					buttonPressMinimumThresholdTime = readTimerMillis() + BUTTON_LONG_PRESS_DURATION_MILLIS - BUTTON_DEBOUNCE_DURATION_MILLIS;
					break;
				}
				else
				{
					userButtonState = BUTTON_IDLE;
					break;
				}										
			}
			else
			{
				userButtonState = BUTTON_PRESSED_DEBOUNCE;
			}
		break;
		case BUTTON_PRESSED:
			if (checkDelayUntil(buttonPressMinimumThresholdTime))
			{
				userButtonState = BUTTON_RELEASE_PENDING;					
				if (switchOverrideMode)
				{
					switchOverrideMode = 0;
					// Swap expected status to force a Auto start stop toogle
					// and make LED blink (first change by BCM, second by the forced one)
					if (readAutoStartStopCurrentStatus())
					{
						autoStarStopExpectedStatus = 0;
					}
					else
					{
						autoStarStopExpectedStatus = 1;
					}
				}
				else
				{
					switchOverrideMode = 1;
				}
				markEEPROMForUpdate = 1;					
				break;
			}
			else
			{
				if (!readButtonRaw()) // Low enabled, button pressed
				{
					userButtonState = BUTTON_PRESSED;
					break;
				}
				else
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
						markEEPROMForUpdate = 1;
					}
					break;
				}
			}
		break;
		case BUTTON_RELEASE_PENDING:
			if (!readButtonRaw())
			{
				userButtonState = BUTTON_RELEASE_PENDING;
				break;
			}
			else
			{
				userButtonState = BUTTON_RELEASE_DEBOUNCE;
				buttonPressMinimumThresholdTime = readTimerMillis() + BUTTON_DEBOUNCE_DURATION_MILLIS;
				break;
			}
			break;
			case BUTTON_RELEASE_DEBOUNCE:
			if (checkDelayUntil(buttonPressMinimumThresholdTime))
			{
				if (readButtonRaw())
				{
						
					userButtonState = BUTTON_IDLE;
					break;
				}
				else
				{
					userButtonState = BUTTON_RELEASE_PENDING;
					break;
				}
			}
			else
			{
				userButtonState = BUTTON_RELEASE_DEBOUNCE;
				break;
			}
		break;
		default:
		break;
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
