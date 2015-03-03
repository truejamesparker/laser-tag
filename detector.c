
#include "supportFiles/interrupts.h"
#include "supportFiles/leds.h"
#include <stdint.h>
#include "isr.h"
#include "stdio.h"
#include "supportFiles/mio.h"
#include "globals.h"
#include "lockoutTimer.h"
#include <stdbool.h>

#define TRANSMITTER_TICK_MULTIPLIER 3	// Call the tick function this many times for each ADC interrupt.
#define LOCKOUT_DURATION 500000
#define DETECTOR_OUTPUT_PIN 11
#define TRUE 1
#define FALSE 0
#define SCALE_FACTOR 4096.00

static volatile bool hit = false;
static double currentPowerValues[NUM_FILTERS];


void addDataToAdcBuffer(uint32_t adcData);
uint32_t isr_removeDataFromAdcBuffer();
uint32_t isr_adcBufferElementCount();

void dector_init() {
	filter_init();
}

detector() {
	uint32_t elementCount = isr_adcBufferElementCount();
	uint32_t rawAdcValue;
	double scaledAdcValue;
	int count=0;

	for(int i=0; i<elementCount; i++) {
		interrupt_disableArmInts();
		// isr_popAdcQueueData();
		rawAdcValue = isr_removeDataFromAdcBuffer();
		interrupts_enableArmInts();
		scaledAdcValue = (double)rawAdcValue/(SCALE_FACTOR);
		filter_addNewInput(scaledAdcValue);
		if(count==10) {
			filter_firFilter();
			for(int j=0; j<NUM_FILTERS; j++) {
				filter_iirFilter(j);
			}
			if(!lockoutTimer_running) {
				detector_hitDetected();
			}
		}
		count++;

	}


}



void dector_clear() {
	hit = false;
}

bool detector_hitDetected() {
	return hit;
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
			
			break;
		case lockout_st:
			
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




