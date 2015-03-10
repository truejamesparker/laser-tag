
#include "supportFiles/interrupts.h"
#include "supportFiles/intervalTimer.h"
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
#include "detector.h"
#include "filter.h"

#define TRANSMITTER_TICK_MULTIPLIER 3	// Call the tick function this many times for each ADC interrupt.
#define LOCKOUT_DURATION 500000
#define DETECTOR_OUTPUT_PIN 11
#define TRUE 1
#define FALSE 0
#define THRESHHOLD_FACTOR 150.00
#define SCALE_FACTOR 4095.00
#define MEDIAN_INDEX 4

static volatile bool hitFlag = false;
static volatile uint16_t hitArray[NUM_FILTERS];
static int count=0;

static double currentPowerValues[NUM_FILTERS];
static uint16_t sortedPowerIndexArray[NUM_FILTERS];


uint32_t isr_removeDataFromAdcBuffer();
uint32_t isr_adcBufferElementCount();

bool detector_hitDetected();
void detector_insertionSort(uint16_t indexArray[], double powerArray[], int elementCount);
bool detector_gotHit();

void detector_init() {
	filter_init();
}

void detector() {
	uint32_t elementCount = isr_adcBufferElementCount();
	uint32_t rawAdcValue;
	double scaledAdcValue;

//	double dur;

	for(uint32_t i=0; i<elementCount; i++) {
		count++;
		interrupts_disableArmInts();
		rawAdcValue = isr_removeDataFromAdcBuffer();
		interrupts_enableArmInts();
		scaledAdcValue = (2*rawAdcValue/(SCALE_FACTOR))-1;
//		printf("Raw ADC: %u\n", rawAdcValue);
//		printf("Scaled ADC: %lf\n", scaledAdcValue);
		filter_addNewInput(scaledAdcValue);
		if(count==10) {
			count=0;

			filter_firFilter();

			for(int j=0; j<NUM_FILTERS; j++) {
				filter_iirFilter(j);
				filter_computePower(j, false, false);
			}

			if(!lockoutTimer_running()) {
				if(detector_gotHit()) {
					lockoutTimer_start();
					hitLedTimer_start();
					hitFlag = true;
				}
			}
		}
	}

}

void detector_clearHit() {
	// unset the hit flag
	hitFlag = false;
}

bool detector_hitDetected() {
	return hitFlag;
}

bool detector_gotHit() {
	double curPower, medianPower, threshholdPower;
	double powerArray[NUM_FILTERS];

	for(int i=0; i<NUM_FILTERS; i++) {
		powerArray[i] = currentPowerValues[i];
	}

	bool flag = false; // determine whether to raise the hitFlag

	for(int i=0; i<NUM_FILTERS; i++) {
		curPower = filter_getCurrentPowerValue(i);
		currentPowerValues[i] = curPower;
	}

	detector_insertionSort(sortedPowerIndexArray, powerArray, NUM_FILTERS);

	uint16_t medianIndex = sortedPowerIndexArray[MEDIAN_INDEX]; // get index of median power
	uint8_t maxIndex = sortedPowerIndexArray[NUM_FILTERS-1]; // get index of highest power

	medianPower = currentPowerValues[medianIndex]; // use sorted index to get median power value
	threshholdPower = medianPower * THRESHHOLD_FACTOR; // compute threshhold power
	// iterate through the power values to see if power > threshhold power
	for (int i=0; i<NUM_FILTERS; i++) {
		if (currentPowerValues[i]>threshholdPower) {
//			printf("curPowerVal: %lf\n", currentPowerValues[i]);
			hitArray[i]++; // CHANGE THIS BACK TO hitArray[i]=1????
			flag = true;
		}
	}

	return flag;
}

void detector_getHitCounts(uint16_t array[]) {
	for(int i=0; i<NUM_FILTERS; i++){
		array[i] = hitArray[i];
	}
}


void detector_insertionSort(uint16_t indexArray[], double powerArray[], int elementCount) {
    int i, j, r;
    double t;
    for (i = 1; i < elementCount; i++) {
        t = powerArray[i];
        r = i;
        for (j = i; j > 0 && t < powerArray[j - 1]; j--) {
            powerArray[j] = powerArray[j - 1];
            indexArray[j] = indexArray[j-1];
        }
        powerArray[j] = t;
        indexArray[j] = r;
    }
}






