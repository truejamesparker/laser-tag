/*
 * detector.c
 *
 *  Created on: Dec 22, 2014
 *      Author: hutch
 */

#include "supportFiles/interrupts.h"
#include "supportFiles/leds.h"
#include <stdint.h>
#include "isr.h"
#include "stdio.h"
#include "supportFiles/mio.h"

#define TRANSMITTER_TICK_MULTIPLIER 3	// Call the tick function this many times for each ADC interrupt.
#define LOCKOUT_DURATION 500000
#define DETECTOR_OUTPUT_PIN 11
#define TRUE 1
#define FALSE 0

void detector_init() {
	mio_setPinAsOutput(DETECTOR_OUTPUT_PIN);
}

bool detector_hit() {
	return TRUE;
}

enum DetectorStates {
	waiting_st,
	lockout_st,
};

// Initialize trigger state
DetectorStates DetectorState = waiting_st;


void dector_tick() {
	static uint32_t lockoutCounter;

	// State actions
	switch(DetectorState) {
		case waiting_st:
			lockoutCounter=0;
			break;
		case lockout_st:
			lockoutCounter++;
			break;
		default:
			break;
	}

	switch(DetectorState) { // Transitions

		case waiting_st: // init state
			if (detector_hit()) {
				DetectorState = lockout_st;
			}
			else {
				DetectorState = waiting_st;
			}
			break;

		case lockout_st: // start debounce counter
			if (lockoutCounter==LOCKOUT_DURATION) {
				DetectorState = waiting_st;
			}
			else {
				DetectorState = lockout_st;
			}
			break;

		default:
			DetectorState = waiting_st;
			break;
	}
}




