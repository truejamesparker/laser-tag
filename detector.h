#ifndef DETECTOR_H_
#define DETECTOR_H_


// Standard initialization function. 
// This would need to call filter_init() and 
// initialize any data structure that you will use.

 void detector_init();
// This runs the entire detector including the decimating 
// FIR-filter, all 10 IIR-filters, power computation, and the threshold-detection scheme.
 void detector();



void detector_tick();


#endif /* DETECTOR_H_ */
