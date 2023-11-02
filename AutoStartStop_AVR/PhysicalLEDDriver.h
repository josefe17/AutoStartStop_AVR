/*
 * PhysicalLEDDriver.h
 *
 * Created: 02/11/2023 20:20:01
 *  Author: josefe
 */ 


#ifndef PHYSICALLEDDRIVER_H_
#define PHYSICALLEDDRIVER_H_

#include <avr/io.h>

// Initializes feedback LED
void initPhysicalLED();
// Turn feedback LED output on
void turnPhysicalLEDOn();
// Turn feedback LED output off
void turnPhysicalLEDOff();
// Controls the LED according to the feedback line status
void forwardBCMSideLEDStatusToPhysicalLED();


#endif /* PHYSICALLEDDRIVER_H_ */