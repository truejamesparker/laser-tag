/*
 * transmitter.h
 *
 *  Created on: Dec 22, 2014
 *      Author: hutch
 */

#ifndef TRANSMITTER_H_
#define TRANSMITTER_H_

 #include <stdint.h>

 // Standard init function.
void transmitter_init();
 
// Starts the transmitter. Does nothing if the transmitter is already running.
void transmitter_run();
 
// Stops the transmitter.
void transmitter_stop();
 
// Returns true if the transmitter is running.
bool transmitter_running();
 
// Sets the frequency number. If this function is called while the
// transmitter is running, the frequency will not be updated until the
// transmitter stops and transmitter_run() is called again.
void transmitter_setFrequencyNumber(uint16_t frequencyNumber);
 
// Standard tick function.
void transmitter_tick();
 
// Tests the transmitter.
void transmitter_runTest();

// Return your own frequency number
uint16_t transmitter_getFrequencyNumber();

// #define TRANSMITTER_OUTPUT_PIN 13	// JF1 (pg. 25 of ZYBO reference manual).


// void transmitter_init();
// void transmitter_stop();
// void transmitter_start();


// void transmitter_setFrequencyNumber(uint16_t frequencyNumber);
// void transmitter_tick();
// void transmitter_runTest();

#endif /* TRANSMITTER_H_ */
