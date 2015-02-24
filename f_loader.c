/*
 * f_loader.c
 *
 *  Created on: Feb 21, 2015
 *      Author: JAMES
 */

#include "f_loader.h"


/////////////////////  TEST FUNCTIONS ////////////////////////

#define JUNK_BUFF_SIZE 60


/*
bool loadSignal(uint16_t length) {
	FILE *f_ptr;
	double value;
	uint16_t i = 0;

	if((f_ptr = fopen("inputSignal.txt", "r")) == NULL) {
		printf("Error opening file!\n");
		return FALSE;
	}

	while (!feof(f_ptr) && fscanf(f_ptr, "%lf", &value) && i<length) {
		queue_overwritePush(&xQ, value);
		i++;
	}

	return TRUE;
}
*/


bool file_loadDataToArray(double *values, int length, const char *fileName) {
	FILE *f_ptr;
	double value;
	int i = 0;

	if((f_ptr = fopen(fileName, "r")) == NULL) {
		printf("Error loading file!\n\r");
		return false;
	}

	while (!feof(f_ptr) && fscanf(f_ptr, "%lf", &value) && i<length) {
		values[i]=value;
		i++;
	}

	fclose(f_ptr);

	return true;
}

bool filter_writeToFile(FILE* f_ptr, double value) {
	if(fprintf(f_ptr, "%10.15lf\n", value)==true) {
		return true;
	}
	else {
		return false;
	}

}
