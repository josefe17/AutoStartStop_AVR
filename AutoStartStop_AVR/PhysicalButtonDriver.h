/*
 * PhysicalButtonDriver.h
 *
 * Created: 02/11/2023 20:23:29
 *  Author: josefe
 */ 


#ifndef PHYSICALBUTTONDRIVER_H_
#define PHYSICALBUTTONDRIVER_H_

#include <avr/io.h>

// Initializes the button hardware.
// To avoid glitches, this function needs to be called the first one
// among GPIO initializations (configures global pullups).
void initPhysicalButton();

// Read the user side button line
uint8_t readPhysicalButtonRaw();

#endif /* PHYSICALBUTTONDRIVER_H_ */