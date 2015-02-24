#ifndef FILTER_H_
#define FILTER_H_

#include <stdint.h>
#include "queue.h"
#include <stdio.h>
#include "globals.h"

#define FILTER_IIR_FILTER_COUNT 10      // You need this many IIR filters.
#define FILTER_FIR_DECIMATION_FACTOR 10	// Filter needs this many new inputs to compute a new output.
#define FILTER_INPUT_PULSE_WIDTH 2000	// This is the width of the pulse you are looking for, in terms of decimated sample count.


// Filtering routines for the laser-tag project.
// Filtering is performed by a two-stage filter, as described below.

// 1. First filter is a decimating FIR filter with a configurable number of taps and decimation factor.
// 2. The output from the decimating FIR filter is passed through a bank of 10 IIR filters. The
// characteristics of the IIR filter are fixed.

// The decimation factor determines how many new samples must be read for each new filter output.
//uint16_t filter_getFirDecimationFactor();

// Must call this prior to using any filter functions.
// Will initialize queues and any other stuff you need to init before using your filters.
// Make sure to fill your queues with zeros after you initialize them.
void filter_init();

// Print out the contents of the xQueue for debugging purposes.
void filter_printXQueue();

// Print out the contents of yQueue for debugging purposes.
void filter_printYQueue();

// Print out the contents of the the specified zQueue for debugging purposes.
void filter_printZQueue(uint16_t filterNumber);

// Use this to copy an input into the input queue (x_queue).
void filter_addNewInput(double x);

// Invokes the FIR filter. Returns the output from the FIR filter. Also adds the output to the y_queue for use by the IIR filter.
double filter_firFilter();

// Use this to invoke a single iir filter. Uses the y_queue and z_queues as input. Returns the IIR-filter output.
double filter_iirFilter(uint16_t filterNumber);

// Use this to compute the power for values contained in a queue.
// If force == true, then recompute everything from scratch.
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch, bool debugPrint);

double filter_getCurrentPowerValue(uint16_t filterNumber);

// Uses the last computed-power values, scales them to the provided lowerBound and upperBound args, returns the index of element containing the max value.
// The caller provides the normalizedArray that will contain the normalized values. indexOfMaxValue indicates the channel with max. power.
void filter_getNormalizedPowerValues(double normalizedArray[], uint16_t* indexOfMaxValue);

bool filter_loadTestData(double *values, const char *fileName);

bool filter_writeToFile(FILE *f_ptr, double value);

void filter_runTest();

void filter_runPowerTest(double inputSignal[], int size);

#endif /* FILTER_H_ */
