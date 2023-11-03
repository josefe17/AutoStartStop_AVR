/*
 * DebugGPIO.h
 *
 * Created: 03/11/2023 12:47:28
 *  Author: josefe
 */ 


#ifndef DEBUGGPIO_H_
#define DEBUGGPIO_H_

#include <avr/io.h>

void initDebugGPIOInput();
void initDebugGPIOInputPullup();
void initDebugGPIOOutput();
void initDebugGPIOOutputHigh();
void initDebugGPIOOutputLow();

uint8_t readDebugGPIO();
void writeDebugGPIO(uint8_t value);


#endif /* DEBUGGPIO_H_ */