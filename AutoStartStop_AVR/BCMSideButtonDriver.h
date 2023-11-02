/*
 * BCMSideButtonDriver.h
 *
 * Created: 02/11/2023 20:22:49
 *  Author: josefe
 */ 


#ifndef BCMSIDEBUTTONDRIVER_H_
#define BCMSIDEBUTTONDRIVER_H_

#include <avr/io.h>

// Initializes GPIO 
void initBCMSideButtonLine();
// Initializes pulse controller
void initBCMSideButtonPulseController();
// Runs pulse controller
void runBCMSideButtonPulseController();

// Hold the start stop button signal pressed
void holdBCMSideButtonLine();
// Set the start stop button signal depressed
void releaseBCMSideButtonLine();
// Set the button signal according to the user button
void forwardPhysicalButtonStatusToBCMSideButtonLine();
// Queue pulse for button line
void queuePulseForBCMSideButtonLine();
// Check if a pulse is ongoing
uint8_t isBCMSideButtonPulseOngoing();

#endif /* BCMSIDEBUTTONDRIVER_H_ */