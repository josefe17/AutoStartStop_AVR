/*
 * BCMSideLEDDriver.h
 *
 * Created: 02/11/2023 20:21:50
 *  Author: josefe
 */ 


#ifndef BCMSIDELEDDRIVER_H_
#define BCMSIDELEDDRIVER_H_

#include <avr/io.h>

// Initializes input LED driver
void initBCMSideLED();

// Check if auto start stop is currently enabled or disabled by BCM side LED signal
// to avoid the device pulsing continuously, ensure this board is powered only while
// the feedback LED is operative
uint8_t readBCMSideLEDLineFiltered();

void initBCMSideLEDFilter();

void runBCMSideLEDFilter();

#endif /* BCMSIDELEDDRIVER_H_ */