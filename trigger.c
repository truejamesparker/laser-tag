/*
 * trigger.c
 *
 *  Created on: Feb 17, 2015
 *      Author: JAMES
 */

#include "trigger.h"
#include "transmitter.h"
#include "supportFiles/mio.h"
#include "supportFiles/buttons.h"
#include <stdio.h>

#define HOLD_TIME 5000
#define TRIGGER_BUTTON 0
#define TRIGGER_INPUT_PIN 10
#define GUN_TRIGGER_PRESSED 1
#define TRUE 1
#define FALSE 0

static bool ignoreGunInput = FALSE;

bool triggerPressed();

void trigger_init() {
  	mio_setPinAsInput(TRIGGER_INPUT_PIN);
  // If the trigger is pressed when trigger_init() is called, assume that the gun is not connected and ignore it.
	if (triggerPressed()) {
		ignoreGunInput = TRUE;
	}
}

bool triggerPressed() {
	return ((!ignoreGunInput & (mio_readPin(TRIGGER_INPUT_PIN) == GUN_TRIGGER_PRESSED)) || 
                (buttons_read() & BUTTONS_BTN0_MASK));
}

 // trigger states
enum TriggerStates {
	start_st,
	init_st,
	debounceTrigger_st,
	debounceRelease_st,
	fire_st,
	complete_st,
};

// Initialize trigger state
TriggerStates TriggerState = start_st;


void trigger_tick() {
	static uint16_t debounceCounter;

	// State actions
	switch(TriggerState) {
		case start_st:
			break;
		case init_st:
			debounceCounter=0;
			break;
		case debounceTrigger_st:
			debounceCounter++;
			break;
		case fire_st:
			break;
		case complete_st:
			break;
		case debounceRelease_st:
			debounceCounter++;
			break;
		default:
			break;
	}

	switch(TriggerState) { // Transitions
		case start_st: // start state (transition to init)
			printf("Initializing Trigger SM\n");
			TriggerState = init_st;
			break;

		case init_st: // init state
			if (triggerPressed()){
				printf("Trigger press detected!\n");
				TriggerState = debounceTrigger_st;
			}
			else {
				TriggerState = init_st;
			}
			break;

		case debounceTrigger_st: // start debounce counter
			if (!triggerPressed()) {
				TriggerState = init_st;
			}
			else if (debounceCounter == HOLD_TIME) {
				printf("Transitioning to fire_st\n");
				TriggerState = fire_st;
				transmitter_run();
			}
			else {
				TriggerState = debounceTrigger_st;
			}
			break;

		case fire_st:
			if(transmitter_running()) {
				TriggerState = fire_st;
			}
			else {
				TriggerState = complete_st;
				printf("Transitioning to complete_st\n");
			}
			break;

		case complete_st:
			printf("Transmit complete!\n");
			if(!triggerPressed()) {
				TriggerState = debounceRelease_st;
				debounceCounter = 0;
			}
			else {
				TriggerState = complete_st;
			}
			break;

		case debounceRelease_st:
			if(!triggerPressed() && debounceCounter >= HOLD_TIME) {
				TriggerState = init_st;
			}
			else {
				TriggerState = debounceRelease_st;
			}

		default:
			TriggerState = start_st;
			break;
	}
}
