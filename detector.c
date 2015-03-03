
#include "supportFiles/interrupts.h"
#include "supportFiles/leds.h"
#include <stdint.h>
#include "isr.h"
#include "hitLedTimer.h"
#include "lockoutTimer.h"
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
#define THRESHHOLD_FACTOR 3.00
#define SCALE_FACTOR 4095.00
#define MEDIAN_INDEX 4

static volatile bool hitFlag = false;
static volatile uint16_t hitArray[NUM_FILTERS];

static double currentPowerValues[NUM_FILTERS];
static double sortedPowerIndexArray[NUM_FILTERS];


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
		scaledAdcValue = (2*(double)rawAdcValue/(SCALE_FACTOR))-1;
		filter_addNewInput(scaledAdcValue);
		if(count==10) {
			filter_firFilter();
			for(int j=0; j<NUM_FILTERS; j++) {
				filter_iirFilter(j);
			}
			if(!lockoutTimer_running()) {
				if(detector_hitDetected()) {
					lockoutTimer_start();
					hitLedTimer_start();
					hitFlag = true;
				}
			}
		}
		count++;
	}
}

void dector_clear() {
	// unset the hit flag
	hitFlag = false;
	// clear hit array
	for(int i=0; i<NUM_FILTERS; i++) {
		hitArray[i] = 0;
	}
}



bool detector_hitDetected() {
	double curPower, medianPower, threshholdPower;
	double powerArray = currentPowerValues;

	for(int i=0; i<NUM_FILTERS; i++) {
		curPower = filter_getCurrentPowerValue(i);
		currentPowerValues[i] = curPower;
	}
	detector_insertionSort(sortedIndexArray, powerArray, NUM_FILTERS);

	uint8_t medianIndex = sortedIndexArray[MEDIAN_INDEX]; // get index of median power
	uint8_t maxIndex = sortedIndexArray[NUM_FILTERS-1]; // get index of highest power

	medianPower = currentPowerValues[medianIndex]; // use sorted index to get median power value
	threshholdPower = medianPower * THRESHHOLD_FACTOR; // compute threshhold power

	// iterate through the power values to see if power > threshhold power
	int flag = false; // determine whether to raise the hitFlag
	for (int i=0; i<NUM_FILTERS; i++) {
		if (currentPowerValues[i]>threshholdPower) {
			hitArray[i] = 1;
			flag = true;
		}
		else
			hitArray[i] = 0;
	}

	if (flag)
		return true;
	else
		return false;
}

void detector_getHitCounts(*array) {
	array = hitArray;
}


void detector_insertionSort (int *indexArray, int *powerArray, int elementCount) {
    int i, j, t, r;
    for (i = 1; i < elementCount; i++) {
        t = powerArray[i];
        r = i;
        for (j = i; j > 0 && t < powerArray[j - 1]; j--) {
            powerArray[j] = powerArray[j - 1];
            indexArray[j] = indexArray[j-1];
        }
        powerArray[j] = t;
        index[j] = r;
    }
}






