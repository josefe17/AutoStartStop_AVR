/*
 * TimerMillis.c
 *
 * Created: 02/11/2023 20:00:45
 *  Author: josefe
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

// Timer postscaler / counter
volatile uint16_t millisCount;

void initTimerMillis()
{
	millisCount = 0;
	GTCCR = (1 << PSR0); // Clear prescaler
	TCCR0A = (1 << WGM01); // CTC
	TCNT0 = 0; // Clear counter
	OCR0A = 125; // Set count
	TIFR = (1 << OCF0A); // Clear IF
	TIMSK = (1 << OCIE0A); // Enable interrupts
	TCCR0B = (1 << CS01) | (1 << CS00); // 64 prescaler
}

uint16_t readTimerMillis()
{
	uint16_t aux;
	TIMSK &= ~(1 << OCIE0A); // Disable timer interrupts
	aux = millisCount;
	TIMSK |= (1 << OCIE0A); // Enable timer interrupts
	return aux;
}

uint8_t checkDelayUntil(uint16_t instant)
{
	return (uint8_t) (readTimerMillis() >= instant);
}

ISR(TIMER0_COMPA_vect)
{
	++millisCount;
}
