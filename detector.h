#ifndef DETECTOR_H_
#define DETECTOR_H_
 
#include <stdint.h>
#include <stdbool.h>
 
#define DETECTOR_TEST_INPUT_PIN 15	 				// JF12 on ZYBO Board. Also bit 0 when reading a bank. Easy to AND.
#define DETECTOR_HIT_THRESHOLD_MULTIPLIER	200	// Just a guess where the max value is around 280 for 200 ms pulse.
#define DETECTOR_HIT_ARRAY_SIZE (FILTER_IIR_FILTER_COUNT)
 
 
typedef uint16_t detector_hitCount_t;
 
// Always have to init things.
void detector_init();
 
// Used to grab debug values during debugging.
queue_t* detector_getDebugQueue();
 
// Runs the entire detector: decimating fir-filter, iir-filters, power-computation, hit-detection.
void detector();
 
// Invoke to determine if a hit has occurred.
bool detector_hitDetected();
 
// Clear the detected hit once you have accounted for it.
void detector_clearHit();
 
// Get the current hit counts.
void detector_getHitCounts(detector_hitCount_t hitArray[]);
 
#endif /* DETECTOR_H_ */