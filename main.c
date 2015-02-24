// #include "transmitter.h"
// #include "supportFiles/intervalTimer.h"
// #include "trigger.h"
#include "filter.h"
// #include "hitLedTimer.h"
// #include "lockoutTimer.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
// #include "supportFiles/mio.h"
// #include "supportFiles/utils.h"
// #include "supportFiles/interrupts.h"
// #include "supportFiles/buttons.h"
// #include "supportFiles/switches.h"
// #include "supportFiles/leds.h"
// #include "isr.h"
#include "f_loader.h"

static double inputSignal[100000];
 
int main() {
	int end = 100000;
	file_loadDataToArray(inputSignal, end, "inputSignal.txt");
	printf("%lf\n", inputSignal[0]);
	printf("%lf\n", inputSignal[end-1]);

	filter_init();
	double normalizedArray[10];
	uint16_t winner;
	uint16_t last_winner=11;

	int player = 0;
	uint16_t j = 0;
	while(j<end) {
		filter_addNewInput(inputSignal[j]);	

		if (j%10==0) {
			filter_firFilter();
			for (int i=0; i<10; i++) {
				filter_iirFilter(i);
				filter_computePower(i, true, false);
				// printf("Current Power: %lf\n", filter_getCurrentPowerValue(i));
			filter_getNormalizedPowerValues(normalizedArray, &winner);
			// if (winner!=last_winner) {
				printf("%u\n", winner);
				// last_winner = winner;
			}
			}


		}
		
		
		// for(int i=0; i<10; i++) {
			// filter_iirFilter(9);
			// filter_computePower(9, false, false);
		// }
		// filter_getNormalizedPowerValues(normalizedArray, &winner);
		// printf("%u\n", winner);
		// printf("Current Power: %lf\n", filter_getCurrentPowerValue(player));
		j++;
	}
	/*
	switches_init();
	intervalTimer_initAll();
	transmitter_init();
	transmitter_setFrequencyNumber(0);
	trigger_init();
	hitLedTimer_init();
	lockoutTimer_init();

	u32 privateTimerTicksPerSecond = interrupts_getPrivateTimerTicksPerSecond();
	interrupts_initAll(true);
	interrupts_enableTimerGlobalInts();
	interrupts_startArmPrivateTimer();
	interrupts_enableSysMonGlobalInts();
	interrupts_enableArmInts();
	
	double time;
	intervalTimer_reset(1);
	intervalTimer_start(1);

	lockoutTimer_start();
//	hitLedTimer_start();
//	while(hitLedTimer_running());
	while(lockoutTimer_running());

	intervalTimer_stop(1);
	intervalTimer_getTotalDurationInSeconds(1, &time);
	printf("Duration: %lf\n", time);

//	transmitter_runTest();
	while(1) {
		uint16_t player_num = (uint16_t)switches_read();
		transmitter_setFrequencyNumber(player_num);
	}*/
}

//static double inputSignal[100001] = {};
// static double iirData[100001];
// static double * inputSignal = (double*)malloc(1001 * sizeof(double));

// int main() {



// 	// filter_loadTestData(iirData, "matlab_filtered.txt");
// 	// filter_loadTestData(inputSignal, "inputSignal.txt");
// 	FILE* f_ptr;
// 	if ((f_ptr = fopen("output.txt", "w"))==NULL) {
// 		printf("Error opening output file!\n\r");
// 	}

// 	queue_runTest();
// 	filter_init();
	

// 	// filter_addNewInput(1);
// 	for(int i=0; i<50; i++) {
// 		double data_point = 0;
// 		double calc_val = 0;
// 		double difference = 0;
		
// 		// calc_val = iirData[i];
// 		filter_addNewInput(inputSignal[i]);
// 		// printf("Data Point %u %10.15lf\n\r", i, inputSignal[i]);
// 		filter_firFilter();
// 		data_point = filter_iirFilter(9);
// 		filter_writeToFile(f_ptr, data_point);

// 		// difference = (data_point - calc_val);
// 		// if(difference<0) {
// 		// 	difference = -difference;
// 		// }

// 		// if (difference>0.001) {
// 		// 	printf("Calc Val: %.10lf   Actual Val: %.10lf\n\r", calc_val, data_point);
// 		// 	printf("Fail: difference is %0.10lf\n\r", difference);
// 		// }
// 	}
// 	fclose(f_ptr);
// }
