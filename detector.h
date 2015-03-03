/*
 * detector.h
 *
 *  Created on: Dec 22, 2014
 *      Author: hutch
 */

#ifndef DETECTOR_H_
#define DETECTOR_H_


// Standard initialization function. 
// This would need to call filter_init() and 
// initialize any data structure that you will use.

 detector_init();
// This runs the entire detector including the decimating 
// FIR-filter, all 10 IIR-filters, power computation, and the threshold-detection scheme.
 detector();



void detector_tick();


#endif /* DETECTOR_H_ */
