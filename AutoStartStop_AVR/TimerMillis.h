/*
 * TimerMillis.h
 *
 * Created: 02/11/2023 20:00:31
 *  Author: josefe
 */ 


#ifndef TIMERMILLIS_H_
#define TIMERMILLIS_H_

#include <avr/io.h>

/* Timer 1 functions */
// Configures the Timer 1 for millis operation
void initTimerMillis();
// Read the Timer 1 for millis operation
uint16_t readTimerMillis();
// Check if that time instant is elapsed or not
uint8_t checkDelayUntil(uint16_t instant);


#endif /* TIMERMILLIS_H_ */