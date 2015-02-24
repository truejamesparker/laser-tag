/*
 * filter.c
 *
 *  Created on: Feb 7, 2015
 *      Author: jwparker
 */

#include "filter.h"


#define XQUEUE_LENGTH 31
#define YQUEUE_LENGTH 11
#define ZQUEUE_LENGTH 11
#define POWER_QUEUE_LENGTH 2000

#define FALSE 0
#define TRUE 1

static queue_t xQ, yQ;
static queue_t zQ[IIR_FILTER_COUNT];
static queue_t powerQ[IIR_FILTER_COUNT];
static double currentPowerValue[IIR_FILTER_COUNT];
// static double normalizedArray[IIR_FILTER_COUNT];

bool loadFIRCoefficients(double *values, uint16_t length);
void initXQueue();
void initYQueue();
void initZQueues();
void initPowerQueues();
bool loadSignal(uint16_t length);

void filter_init() {
  // Init queues and fill them with 0s.
  initXQueue();  // Call queue_init() on xQueue and fill it with zeros.
  initYQueue();  // Call queue_init() on yQueue and fill it with zeros.
  initZQueues(); // Call queue_init() on all of the zQueues and fill each z queue with zeros.
  initPowerQueues();
}

void initXQueue() {                          // x is the queue with the input history for the queue.
	 queue_init(&xQ, XQUEUE_LENGTH);
	 for (int i=0; i<XQUEUE_LENGTH; i++) {  // Start out with an empty queue (all zeroes).
		queue_overwritePush(&xQ, 0.0);
	 }
 }

void initYQueue() {                         // x is the queue with the input history for the queue.
	 queue_init(&yQ, YQUEUE_LENGTH);
	 for (int i=0; i<YQUEUE_LENGTH; i++) {  // Start out with an empty queue (all zeroes).
		queue_overwritePush(&yQ, 0.0);
		}
}

void initZQueues() {
	for(int j=0; j<IIR_FILTER_COUNT; j++) {
		queue_init(&zQ[j], ZQUEUE_LENGTH);
		for (int i=0; i<ZQUEUE_LENGTH; i++) {
			queue_overwritePush(&(zQ[j]), 0.0);
		}
	 }
}

void initPowerQueues() {
	for(int j=0; j<IIR_FILTER_COUNT; j++) {
		queue_init(&powerQ[j], POWER_QUEUE_LENGTH);
		for (int i=0; i<POWER_QUEUE_LENGTH; i++) {
			queue_overwritePush(&(powerQ[j]), 0.0);
		}
	}
}


// Print out the contents of the xQueue for debugging purposes.
void filter_printXQueue() {
	queue_print(&xQ);
}

// Print out the contents of yQueue for debugging purposes.
void filter_printYQueue() {
	queue_print(&yQ);
}

// Print out the contents of the the specified zQueue for debugging purposes.
void filter_printZQueue(uint16_t filterNumber) {
	queue_print(&zQ[filterNumber]);
}

// Use this to copy an input into the input queue (x_queue).
void filter_addNewInput(double x) {
	queue_overwritePush(&xQ, x);
}

double filter_firFilter() {
	double y=0.0;
	for (int i=0; i<FIR_COEF_COUNT; i++) {
		y += queue_readElementAt(&xQ, i) * firCoefficients[FIR_COEF_COUNT-1-i];  // iteratively adds the (b * input) products.
	}
	queue_overwritePush(&yQ, y);
	// printf("%10.15lf\n\r", y);
	return y;
}

// Use this to invoke a single iir filter. Uses the y_queue and z_queues as input. Returns the IIR-filter output.
double filter_iirFilter(uint16_t filterNumber) {
	double z = 0.0f;
	double s1 = 0.0f;
	double s2 = 0.0f;
	for (int i=0; i<IIR_COEF_COUNT; i++) {
		s1 += queue_readElementAt(&yQ, i) * iirBCoefficients[filterNumber][IIR_COEF_COUNT-1-i];  // start at index 0
		// printf("%.15lf\n\r", iirBCoefficients[filterNumber][IIR_COEF_COUNT-1-i]);
	}

	for (int i=1; i<IIR_COEF_COUNT; i++) {
		s2 += queue_readElementAt(&(zQ[filterNumber]), i) * iirACoefficients[filterNumber][IIR_COEF_COUNT-i];  // start at index 1
		// printf("%.15lf\n\r", iirACoefficients[filterNumber][IIR_COEF_COUNT-i]);
	}

	z = (s1 - s2);
	queue_overwritePush(&(zQ[filterNumber]), z);
	// printf("%10.15lf\n\r", z);
	return z;

}


void filter_runTest() {
	printf("****** filter_runTest ******\n\r");
	filter_init();
	queue_overwritePush(&zQ[0], 1.0);
	for(int i=0; i<30; i++) {
		filter_iirFilter(0);
		queue_overwritePush(&zQ[0], 0.0);
	}
	// queue_print(&xQ);
}


// Use this to compute the power for values contained in a queue.
// If force == true, then recompute everything from scratch.
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch, bool debugPrint) {
	double zVal = queue_readElementAt(&(zQ[filterNumber]), ZQUEUE_LENGTH-1);
	double newPower = zVal * zVal; // square to get power; x*x is faster than pow(x,2)
	double oldPower = queue_readElementAt(&powerQ[filterNumber], 0);
	// VERIFY THIS IS THE CORRECT IMPLEMENTATION!
	if (forceComputeFromScratch) {
		double totalPower = 0;
		// iteratively add each power value
		for (int i=0; i<POWER_QUEUE_LENGTH; i++) {
			totalPower += queue_readElementAt(&powerQ[filterNumber], i);
		}
		currentPowerValue[filterNumber] = totalPower + newPower - oldPower; // add the total, subtract old power, add new power
	}
	else {
		currentPowerValue[filterNumber] += (newPower - oldPower); // running average implementation
	}
	queue_overwritePush(&powerQ[filterNumber], newPower);

	if (debugPrint) {
		printf("%.8lf\n", newPower);
	}

	return currentPowerValue[filterNumber];
}

double filter_getCurrentPowerValue(uint16_t filterNumber) {
	return currentPowerValue[filterNumber];
}

// Uses the last computed-power values, scales them to the provided lowerBound and upperBound args, returns the index of element containing the max value.
// The caller provides the normalizedArray that will contain the normalized values. indexOfMaxValue indicates the channel with max. power.
void filter_getNormalizedPowerValues(double normalizedArray[], uint16_t* indexOfMaxValue) {
	double highPower = 0.0;
	for (int i=0; i<IIR_FILTER_COUNT; i++) {
		if (currentPowerValue[i] > highPower) {
			highPower = currentPowerValue[i];
			*indexOfMaxValue = i;
		}
	}
	for (int i=0; i<IIR_FILTER_COUNT; i++) {
		normalizedArray[i] = currentPowerValue[i]/highPower;
	}
}
