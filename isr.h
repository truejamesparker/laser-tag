#ifndef ISR_H_
#define ISR_H_

// isr provides the isr_function() where you will place functions that require accurate timing.
// A buffer for storing values from the Analog to Digital Converter (ADC) is implemented in isr.c

// Performs inits for anything in isr.c
void isr_init();

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function();

// This removes a value from the ADC buffer.
uint32_t isr_removeDataFromAdcBuffer();

// This returns the number of values in the ADC buffer.
uint32_t isr_adcBufferElementCount();

#endif /* ISR_H_ */