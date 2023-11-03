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
void initPhysicalButtonLine();

// Configure the input button FSM variables
void initPhysicalButtonController();

// Run the user side button FSM machine
void runPhysicalButtonController();

// Read the user side button line
uint8_t readPhysicalButtonRaw();

// Check if a valid short press has been produced
uint8_t checkPhysicalButtonShortPress();

// Check if a valid long press has been produced
uint8_t checkPhysicalButtonLongPress();

// Check if a long press release is pending (button is still hold after long press)
uint8_t checkPhysicalButtonLongPressReleasePending();

#endif /* PHYSICALBUTTONDRIVER_H_ */