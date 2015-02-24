/*
 * transmitter.c
 *
 *  Created on: Dec 22, 2014
 *      Author: hutch
 */

#include "transmitter.h"
#include <stdint.h>
#include "supportFiles/buttons.h"
#include "supportFiles/switches.h"
#include "supportFiles/mio.h"
#include "supportFiles/utils.h"
#include <stdio.h>
#include <stdbool.h>

#define TRANSMITTER_OUTPUT_PIN 13
#define TRANSMITTER_HIGH_VALUE 1
#define TRANSMITTER_LOW_VALUE 0
#define TRANSMIT_DURATION 20000


static volatile uint16_t freq_num = 0;
static double freqs[10] = {1.1111e3, 1.3889e3, 1.7241e3, 2.0e3, 2.2727e3, 2.6316e3, 2.9412e3, 3.3333e3, 3.5714e3, 3.8462e3};
static uint16_t halfTicks[10] = {45, 36, 29, 25, 22, 19, 17, 15, 14, 13};
static volatile bool runTransmit = false;


void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
	if (frequencyNumber<10) {
		freq_num = frequencyNumber;
	}
	else {
		freq_num = 9;
	}
}

 // Standard init function.
void transmitter_init() {
	buttons_init();
	mio_init(false);  // false disables any debug printing if there is a system failure during init.
  	mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN);                   // Configure the pin to be an output.
  	mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE); // Write to the output pin.
}
 
// Starts the transmitter. Does nothing if the transmitter is already running.
void transmitter_run() {
	runTransmit = true;
}
 
// Stops the transmitter.
void transmitter_stop() {
	runTransmit = false;
}
 
// Returns true if the transmitter is running.
bool transmitter_running() {
	return runTransmit;
}

// Transmitter states
enum TransmitStates {
	init_st,
	one_st,
	zero_st,
	complete_st,
};

// Initialize transmitter state
TransmitStates TransmitState = init_st;


void transmitter_tick() {
	static uint16_t ticksPerHalfDuty;
	static uint16_t dutyCounter;
	static uint16_t fireCounter;

	// State actions
	switch(TransmitState) {

		case init_st:
			dutyCounter = 0;
			fireCounter = 0;
			break;
		case one_st:
			dutyCounter++;
			fireCounter++;
			break;
		case zero_st:
			dutyCounter++;
			fireCounter++;
			break;
		case complete_st:
			break;
		default:
			break;
	}

	switch(TransmitState) { // Transitions

		case init_st: // init state
			if (transmitter_running()) {
				// printf("Freq: %f\n", freqs[freq_num]);
				ticksPerHalfDuty = halfTicks[freq_num];
				TransmitState = one_st;
				mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_HIGH_VALUE);
			}
			else {
				TransmitState = init_st;
			}
			break;

		case one_st: // write one to
			if (fireCounter == TRANSMIT_DURATION) {
				// printf("Transitioning to complete_st\n");
				TransmitState = complete_st;
				transmitter_stop();
				mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE);
			}
			else if (dutyCounter == ticksPerHalfDuty) {
				// printf("Transitioning to zero_st\n");
				TransmitState = zero_st;
				mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE);
				dutyCounter = 0;
			} 
			else {
				TransmitState = one_st;
			}
			break;

		case zero_st:
			if (fireCounter == TRANSMIT_DURATION) {
				// printf("Transitioning to complete_st\n");
				TransmitState = complete_st;
				transmitter_stop();
			}
			else if (dutyCounter == ticksPerHalfDuty) {
				// printf("Transitioning to one_st\n");
				TransmitState = one_st;
				mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_HIGH_VALUE);
				dutyCounter = 0;
			}
			else {
				TransmitState = zero_st;
			}
			break;

		case complete_st:
			// printf("Transmit complete!\n");
			TransmitState = init_st;
			break;

		default:
			TransmitState = init_st;
			break;
	}
}

void transmitter_runTest() {

	transmitter_init();

	for (int i=0; i<10; i++) {
		printf("Transmitting at frequency %d\n", i);
		transmitter_setFrequencyNumber(i);
		transmitter_run();
		while(transmitter_running());
		utils_msDelay(500);
	}
	printf("transmit_runTest complete!\n");
}
