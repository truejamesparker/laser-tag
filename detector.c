
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

#define THRESHHOLD_FACTOR 200.00 // the "fudge factor" used doing relative powre comparisons
#define SCALE_FACTOR 4095.00 // the scaling factor of the ADC values
#define MEDIAN_INDEX 4 // the standard median index (out of an array of 10)

static volatile bool hitFlag = false; // a boolean flag indicating if a hit has just been detected
static volatile uint16_t hitArray[NUM_FILTERS]; // histogram array of detected hits
static volatile int count=0; // used for tracking decimation

static double currentPowerValues[NUM_FILTERS]; // the current power values loaded from filter functions
static uint16_t sortedPowerIndexArray[NUM_FILTERS]; // sorts player numbers by highest channel power
static double debugArray[NUM_FILTERS]; // used when testing the detect_hit algorithm


// fucntions prototypes
uint32_t isr_removeDataFromAdcBuffer();
uint32_t isr_adcBufferElementCount();
bool detector_hitDetected();
void detector_insertionSort(uint16_t indexArray[], double powerArray[], int elementCount);
bool detect_hit(bool debug);

// initialization
void detector_init() {
	filter_init();
}

// the primary detector function
void detector() {
	uint32_t elementCount = isr_adcBufferElementCount(); // determine # of elements in adc buffer
	uint32_t rawAdcValue;
	double scaledAdcValue;

	for(uint32_t i=0; i<elementCount; i++) {
		count++; // increment the decimation counter
		interrupts_disableArmInts(); // tempoarily disable interrupts while unloading the adc buffer
		rawAdcValue = isr_removeDataFromAdcBuffer();
		interrupts_enableArmInts(); // re-enable the interrupts
		scaledAdcValue = (2*rawAdcValue/(SCALE_FACTOR))-1;
//		printf("Raw ADC: %u\n", rawAdcValue);
//		printf("Scaled ADC: %lf\n", scaledAdcValue);
		filter_addNewInput(scaledAdcValue);
		if(count==10) {
			count=0; // reset the decimation counter
			filter_firFilter();

			for(int j=0; j<NUM_FILTERS; j++) {
				filter_iirFilter(j);
				filter_computePower(j, false, false);
			}

			if(!lockoutTimer_running()) {
				if(detect_hit(false)) {
					lockoutTimer_start(); // start lockout proceedures
					hitLedTimer_start(); // start the hit led
					hitFlag = true; // set the global hit flag to true
				}
			}
		}
	}
}

// reset the global hit flag
void detector_clearHit() {
	hitFlag = false;
}

// return the global hit flat
bool detector_hitDetected() {
	return hitFlag;
}

// the detection alogrithm
bool detect_hit(bool debug) {
	double curPower, medianPower, threshholdPower;
	double powerArray[NUM_FILTERS];
	bool flag = false; // initialize hit flag to false

	// get power values using filter_getCurrentPowerValue() unless in debug mode
	for(int i=0; i<NUM_FILTERS; i++) {
		if(!debug)
			curPower = filter_getCurrentPowerValue(i);
		else
			curPower = debugArray[i];
		currentPowerValues[i] = curPower;
	}

	// copy currentPowerValues into powerArray (used in the sorting algorithm)
	for(int i=0; i<NUM_FILTERS; i++) {
		powerArray[i] = currentPowerValues[i];
	}

	// sort the powerArray (copy of currentPowerValues) and create the sortedPowerIndexArray
	detector_insertionSort(sortedPowerIndexArray, powerArray, NUM_FILTERS);

	uint16_t medianIndex = sortedPowerIndexArray[MEDIAN_INDEX]; // get index of median power
	uint8_t maxIndex = sortedPowerIndexArray[NUM_FILTERS-1]; // get index of highest power

	medianPower = currentPowerValues[medianIndex]; // use sorted index to get median power value
	threshholdPower = medianPower * THRESHHOLD_FACTOR; // compute threshhold power
	
	// keep track of the player with the "closesest" hit
	// this ensures that we only register one hit per iteration
	double highPower = 0;
	uint16_t highIndex = 0;

	// iterate through the power values to see if power > threshhold power
	for (int i=0; i<NUM_FILTERS; i++) {
		if (currentPowerValues[i]>threshholdPower) {
			if (currentPowerValues[i]>highPower) {
				highPower = currentPowerValues[i];
				highIndex = i;
			}
			flag = true; // assert the local hit flag
		}
	}
	if(flag)
		hitArray[highIndex]++; // increment the hit histogram if a hit was detected
	return flag; // alerts detector() that a hit was detected; different from the global hitFlag
}


// essentially copies the histogram hit array
void detector_getHitCounts(uint16_t array[]) {
	for(int i=0; i<NUM_FILTERS; i++){
		array[i] = hitArray[i];
	}
}

// insertion sorting algorithm
// powerArray will be sorted from highest to lowest power
// indexArray will keep track of the original indexes of the now sorted powerArray
// elementCount is the length of the powerArray
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

// this test requires a vector of power values and the computed mean of the data. The tests then 
// runs detect_hit in debug mode on the test data and checks the mean against the computed "golden mean". 
// The test also checks whether there was a hit on the first test vector but not one on second vector.
void detector_runTest(double testVecTrue[], double testVecFalse[], double goldenMean1, double goldenMean2) {
	
	printf("Beginning Test 1...\n\r");

	// load the debug array with the test vector values
	for (int i=0; i<NUM_FILTERS; i++) {
		debugArray[i] = testVecTrue[i];
	}

	// check if hit was detected using hit-true data
	if(!detect_hit(true))
		printf("Test Fail! Hit not detected with testVecTrue\n");
	else
		printf("Success! Hit detected!\n");
	
	double calc_goldenMean1 = currentPowerValues[MEDIAN_INDEX];

	if(goldenMean1!=calc_goldenMean1)
		printf("Test Fail! Means not equal!\n");
	

	printf("Golden Mean 1: %.2lf	Calculated Mean 1: %.2lf\n", goldenMean1, calc_goldenMean1);
	printf("Beginning Test 2...\n\r");

	// 
	for (int i=0; i<NUM_FILTERS; i++) {
		debugArray[i] = testVecFalse[i];
	}

	// check if hit was detected using hit-false data
	if(detect_hit(true))
		printf("Test Fail! Hit detected with testVecFalse\n");
	else
		printf("Success! Hit not detected!\n");
	
	double calc_goldenMean2 = currentPowerValues[MEDIAN_INDEX];

	// compare calculated mean with evaluated mean
	if(goldenMean2!=calc_goldenMean2)
		printf("Test Fail! Means not equal!\n");

	printf("Golden Mean 2: %.2lf	Calculated Mean 2: %.2lf\n", goldenMean2, calc_goldenMean2);

	printf("detector_runTest completed.\n");
}
