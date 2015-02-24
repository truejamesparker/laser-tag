/*
 * isr.h
 *
 *  Created on: Jan 2, 2015
 *      Author: hutch
 */

#ifndef ISR_H_
#define ISR_H_

// Removes data from the ADC queue.
//queue_data_t isr_popAdcQueueData();

// Lets you know how many elements are in the queue.
//queue_size_t isr_adcQueueElementCount();

// Gives you the total of ADC samples that have been taken thus far.
uint64_t isr_getTotalAdcSampleCount();

// Provided so that you can init any necessary data structures.
void isr_init();

// This is the function that the code that is invoked when a timer interrupt occurs.
void isr_function();

#endif /* ISR_H_ */
