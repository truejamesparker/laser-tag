/*
 * hitLedTimer.c
 *
 *  Created on: Feb 21, 2015
 *      Author: JAMES
 */

#include "hitLedTimer.h"
#include "supportFiles/interrupts.h"
#include "supportFiles/leds.h"
#include <stdint.h>
#include <stdbool.h>
#include "isr.h"
#include "stdio.h"
#include "supportFiles/mio.h"

#define LED_DURATION 50000
#define LED_OUTPUT_PIN 11
#define LED_0_MASK 0x01
#define ON 1
#define OFF 0

static bool runHit = false;
// Need to init things.
void hitLedTimer_init() {
	leds_init(false);
	mio_setPinAsOutput(LED_OUTPUT_PIN);
}

// Calling this starts the timer.
void hitLedTimer_start() {
	runHit = true;
}

void hitLedTimer_stop() {
	runHit = false;
}

// Returns true if the timer is currently running.
bool hitLedTimer_running() {
	return runHit;
}

// Standard tick function.
enum LedTimerStates {
	waiting_st,
	on_st,
};

// Initialize trigger state
LedTimerStates LedTimerState = waiting_st;


void hitLedTimer_tick() {
	static uint16_t ledCounter;

	// State actions
	switch(LedTimerState) {
		case waiting_st:
			ledCounter=0;
			break;
		case on_st:
			ledCounter++;
			break;
		default:
			break;
	}

	switch(LedTimerState) { // Transitions

		case waiting_st: // init state
			if (hitLedTimer_running()) {
				LedTimerState = on_st;
				leds_write(LED_0_MASK);
				mio_writePin(LED_OUTPUT_PIN, ON);
			}
			else {
				LedTimerState = waiting_st;
			}
			break;

		case on_st: // start debounce counter
			if (ledCounter>LED_DURATION) {
				LedTimerState = waiting_st;
				leds_write(OFF);
				mio_writePin(LED_OUTPUT_PIN, OFF);
				hitLedTimer_stop();
			}
			else {
				LedTimerState = on_st;
			}
			break;

		default:
			LedTimerState = waiting_st;
			break;
	}
}


