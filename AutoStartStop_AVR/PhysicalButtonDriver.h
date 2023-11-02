/*
 * PhysicalButtonDriver.h
 *
 * Created: 02/11/2023 20:23:29
 *  Author: josefe
 */ 


#ifndef PHYSICALBUTTONDRIVER_H_
#define PHYSICALBUTTONDRIVER_H_

#include <avr/io.h>

// Read the user side button line
uint8_t readButtonRaw();

#endif /* PHYSICALBUTTONDRIVER_H_ */