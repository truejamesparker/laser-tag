
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
#define THRESHHOLD_FACTOR 200.00
#define SCALE_FACTOR 4095.00
#define MEDIAN_INDEX 4

static volatile bool hitFlag = false;
static volatile uint16_t hitArray[NUM_FILTERS];
static int count=0;

static double currentPowerValues[NUM_FILTERS];
static uint16_t sortedPowerIndexArray[NUM_FILTERS]; // Sort indexes of players by highest channel power
static double debugArray[NUM_FILTERS];

uint32_t isr_removeDataFromAdcBuffer();
uint32_t isr_adcBufferElementCount();

bool detector_hitDetected();
void detector_insertionSort(uint16_t indexArray[], double powerArray[], int elementCount);
bool detect_hit(bool debug);

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
				if(detect_hit(false)) {
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

bool detect_hit(bool debug) {
	double curPower, medianPower, threshholdPower;
	double powerArray[NUM_FILTERS];
	bool flag = false; // determine whether to raise the hitFlag

	for(int i=0; i<NUM_FILTERS; i++) {
		if(!debug)
			curPower = filter_getCurrentPowerValue(i);
		else
			curPower = debugArray[i];
		currentPowerValues[i] = curPower;
	}

	for(int i=0; i<NUM_FILTERS; i++) {
		powerArray[i] = currentPowerValues[i];
	}

	detector_insertionSort(sortedPowerIndexArray, powerArray, NUM_FILTERS);

	uint16_t medianIndex = sortedPowerIndexArray[MEDIAN_INDEX]; // get index of median power
	uint8_t maxIndex = sortedPowerIndexArray[NUM_FILTERS-1]; // get index of highest power

	medianPower = currentPowerValues[medianIndex]; // use sorted index to get median power value
	threshholdPower = medianPower * THRESHHOLD_FACTOR; // compute threshhold power
	// iterate through the power values to see if power > threshhold power
	double highPower = 0;
	uint16_t highIndex = 0;

	for (int i=0; i<NUM_FILTERS; i++) {
		if (currentPowerValues[i]>threshholdPower) {
			if (currentPowerValues[i]>highPower) {
				highPower = currentPowerValues[i];
				highIndex = i;
			}
			flag = true;
		}
	}
	if(flag) {
		hitArray[highIndex]++;
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

void detector_runTest(double testVecTrue[], double testVecFalse[], double goldenMean1, double goldenMean2) {
	
	printf("Beginning Test 1...\n");

	for (int i=0; i<NUM_FILTERS; i++) {
		debugArray[i] = testVecTrue[i];
	}

	if(!detect_hit(true))
		printf("Test Fail! Hit not detected with testVecTrue\n");
	
	double calc_goldenMean1 = currentPowerValues[MEDIAN_INDEX];

	if(goldenMean1!=calc_goldenMean1)
		printf("Test Fail! Means not equal!\n");

	printf("Golden Mean 1: %.2lf	Calculated Mean 1: %.2lf\n", goldenMean1, calc_goldenMean1);
	printf("Beginning Test 2...\n");

	for (int i=0; i<NUM_FILTERS; i++) {
		debugArray[i] = testVecFalse[i];
	}

	if(detect_hit(true))
		printf("Test Fail! Hit detected with testVecFalse\n");
	
	double calc_goldenMean2 = currentPowerValues[MEDIAN_INDEX];

	if(goldenMean2!=calc_goldenMean2)
		printf("Test Fail! Means not equal!\n");
	printf("Golden Mean 2: %.2lf	Calculated Mean 2: %.2lf\n", goldenMean2, calc_goldenMean2);

	printf("detector_runTest completed.\n");
}