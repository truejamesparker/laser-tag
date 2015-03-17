#include <stdio.h>
#include "supportFiles/leds.h"
#include "supportFiles/globalTimer.h"
#include "supportFiles/interrupts.h"
#include "supportFiles/intervalTimer.h"
#include <stdbool.h>
#include "queue.h"
#include "xparameters.h"
#include "filter.h"
#include "histogram.h"
#include "detector.h"
#include "transmitter.h"
#include <stdlib.h>
#include "supportFiles/buttons.h"
#include "supportFiles/switches.h"
#include "isr.h"
#include "supportFiles/mio.h"
#include <string.h>
#include "supportFiles/display.h"
#include "trigger.h"
#include "lockoutTimer.h"
#include "hitLedTimer.h"


#define TOTAL_RUNTIME_TIMER 1
//#define ISR_CUMULATIVE_TIMER INTERRUPT_CUMULATIVE_ISR_INTERVAL_TIMER_NUMBER
#define ISR_CUMULATIVE_TIMER 0  // Not currently defined in the student's version of the code.
#define MAIN_CUMULATIVE_TIMER 2

#define SYSTEM_TICKS_PER_HISTOGRAM_UPDATE 50000	// Effectively 2 times per second.

// Tries to squeeze a little more into 4 characters by removing the e part of the exponent.
#define EXPONENT_CHARACTER 'e'
void trimLabel(char label[]) {
  uint16_t len = strlen(label);  // Get the length of the label.
  bool found_e = false;  // You looking for the exponent character.
  uint16_t e_index = 0;  // will point to the exponent character when completed.
  // Look for the e and keeps its index.
  for (int i=0; i<len; i++) {
    if (label[i] == EXPONENT_CHARACTER) {
      found_e = true;  // found the exponent character.
      e_index = i;     // Note its position.
      break;
    }
  }
  // If you found the "e", just copy over it.
  if (found_e) {
    for (int j=e_index; j<len; j++) {
      label[j] = label[j+1];
    }
  }
}

static uint32_t countInterruptsViaInterruptsIsrFlag = 0;

// Prints out various run-time statistics on the TFT display.
// Assumes the following:
// main is keeping track of detected interrupts with countInterruptsViaInterruptsIsrFlag,
// interval_timer(0) is the cumulative run-time of the ISR,
// interval_timer(1) is the total run-time,
// interval_timer(2) is the time spent in main running the filters, updating the display, and so forth.
// No comments in the code, the print statements are self-explanatory.
void printRunTimeStatistics() {
  display_setTextSize(1);
  display_setTextColor(DISPLAY_WHITE);
  display_setCursor(0, 0);
  display_fillScreen(DISPLAY_BLACK);
  display_print("Elements remaining in ADC queue:");
  display_print(isr_adcBufferElementCount());
  display_println(); display_println();
  double runningSeconds, isrRunningSeconds, mainLoopRunningSeconds;
  intervalTimer_getTotalDurationInSeconds(TOTAL_RUNTIME_TIMER, &runningSeconds);
  display_print("Measured run time in seconds: ");
  display_print(runningSeconds);
  display_println(); display_println();
  intervalTimer_getTotalDurationInSeconds(ISR_CUMULATIVE_TIMER, &isrRunningSeconds);
  display_print("Cumulative run time in timerIsr: ");
  display_print(isrRunningSeconds); display_print(" (");
  display_print(isrRunningSeconds/runningSeconds*100);
  display_println("%)"); display_println();
  intervalTimer_getTotalDurationInSeconds(MAIN_CUMULATIVE_TIMER, &mainLoopRunningSeconds);
  display_print("Cumulative run-time in main loop: ");
  display_print(mainLoopRunningSeconds); display_print(" (");
  display_print((mainLoopRunningSeconds/runningSeconds)*100);
  display_println("%)"); display_println();
  uint32_t interruptCount = interrupts_isrInvocationCount();
  display_print("Total interrupts:            "); display_println(interruptCount); display_println();
  display_print("Interrupts detected in main: ");
  display_println(countInterruptsViaInterruptsIsrFlag); display_println();
  display_print("Detected interrupts in main: ");
  display_print(((double) countInterruptsViaInterruptsIsrFlag / (double) interruptCount)*100); display_print("%");
}

// This mode runs continously until btn3 is pressed.
// When btn3 is pressed, it exits and prints performance information to the TFT.
// During operation, it continously displays that received power on each channel, on the TFT.
void continuousPowerMode() {
  // Lots of init's.
  display_init();
  intervalTimer_initAll();
  histogram_init();
  leds_init(true);
  transmitter_init();
  detector_init();
  filter_init();
  isr_init();
  // Init all interrupts (but does not enable the interrupts at the devices).
  // Prints an error message if an internal failure occurs because the argument = true.
  interrupts_initAll(true);
  interrupts_enableTimerGlobalInts();		// Allows the timer to generate interrupts.
  interrupts_startArmPrivateTimer();		// Start the private ARM timer running.


  uint16_t histogramSystemTicks = 0;		// Only update the histogram display every so many ticks.
  double normalizedPowerValues[FILTER_IIR_FILTER_COUNT];// Use this to store normalized power values for the histogram.
  uint16_t indexOfMaxValue;		// Keep track of the index of the maximum value.
  intervalTimer_reset(ISR_CUMULATIVE_TIMER);	// Used to measure ISR execution time.
  intervalTimer_reset(TOTAL_RUNTIME_TIMER);	// Used to measure total program execution time.
  intervalTimer_reset(MAIN_CUMULATIVE_TIMER);	// Used to measure main-loop execution time.
  intervalTimer_start(TOTAL_RUNTIME_TIMER);	// Start measuring total execution time.
  interrupts_enableArmInts();		// The ARM will start seeing interrupts after this.

  printf("Finished init for continuousPowerMode\n");

  while (!(buttons_read() & BUTTONS_BTN3_MASK)) {	// Run until you detect btn3 pressed.
    if (interrupts_isrFlagGlobal) {		// Only do something if an interrupt has occurred.
      transmitter_run();							// Run the transmitter continuously, stops after one period if you don't constantly invoke this.
      intervalTimer_start(MAIN_CUMULATIVE_TIMER);					// Measure run-time when you are doing something.
      histogramSystemTicks++;					// Keep track of ticks so you know when to update the histogram.
      countInterruptsViaInterruptsIsrFlag++;	// Keep track of the interrupt-count based on the global flag.
      interrupts_isrFlagGlobal = 0;						// Reset the global flag.
      detector();												// Run filters, compute power, etc.
      filter_getNormalizedPowerValues(normalizedPowerValues, &indexOfMaxValue);	// This normalizes power between 1 and 0.
      if (histogramSystemTicks >= SYSTEM_TICKS_PER_HISTOGRAM_UPDATE) {	// If enough ticks have transpired, update the histogram.
	for (int i=0; i<FILTER_IIR_FILTER_COUNT; i++) {									// Update across all filters.
	  // The height of the histogram bar depends upon the normalized value.
	  histogram_data_t histogramBarValue = ((double) (HISTOGRAM_MAX_BAR_DATA_IN_PIXELS)) * normalizedPowerValues[i];
	  // You can have a dynamic label at the top of the bar.
	  char label[HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS];	// Get a buffer for the label.
	  // Create the label, based upon the actual power value.
	  if (snprintf(label, HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS, "%0.0e", filter_getCurrentPowerValue(i)) == -1)
	    printf("Error: snprintf encountered an error during conversion.\n\r");
	  // Pull out the 'e' from the exponent to make better use of your characters.
	  trimLabel(label);
	  // Have the bar value and the label, send the data to the histogram.
	  if (!histogram_setBarData(i, histogramBarValue, label)) {
	    // If returns false, histogram_setBarData() is not happy. Print out some information.
	    printf("Error:histogram_setBarData() histogramBarValue(%d) out of range.\n\r", histogramBarValue);
	    printf("Provided normalizedPowerValue[%d]:%lf\n\r", i, normalizedPowerValues[i]);
	    printf("Dumping current and normalized power values.\n\r");
	    for (int tmp_i=0; tmp_i<FILTER_IIR_FILTER_COUNT; tmp_i++) {
	      printf("currentPowerValue[%d]:%lf\n\r", tmp_i, filter_getCurrentPowerValue(tmp_i));
	      printf("normalizedPowerValue[%d]:%lf\n\r", tmp_i, normalizedPowerValues[tmp_i]);
	    }
	  }
	}
	histogram_updateDisplay();	// Finally, render the histogram on the TFT.
	histogramSystemTicks = 0;		// Reset the tick count and wait for the next update time.
	uint16_t switchValue = switches_read();	// Read the switches and switch frequency as required.
	// Note that Brian sends the coefficients with the min. frequency at 0, max. frequency at 9. Transmitter does likewise.
	transmitter_setFrequencyNumber(switchValue);
      }
      intervalTimer_stop(2);
    }
  }
  interrupts_disableArmInts();
  printRunTimeStatistics();
}

void computeNormalizedHitValues(double normalizedHitValues[], uint16_t hitArray[]) {
  // First, find the indicies of the min. and max. value in the currentPowerValue array.
  uint16_t maxIndex = 0;
  for (int i=0; i<FILTER_IIR_FILTER_COUNT; i++) {
    if (hitArray[i] > hitArray[maxIndex])
      maxIndex = i;
  }
  double maxHitValue = (double) hitArray[maxIndex];
  // Normalize everything between 0.0 and 1.0.
  for (int i=0; i<FILTER_IIR_FILTER_COUNT; i++)
    normalizedHitValues[i] = (double) hitArray[i] / maxHitValue;
}

// Game-playing mode. Each shot is registered on the histogram on the TFT.
void shooterMode() {
  // Lots of init's.
  display_init();
  intervalTimer_init(0);
  intervalTimer_init(1);
  intervalTimer_init(2);
  histogram_init();
  leds_init(true);
  transmitter_init();
  detector_init();
  filter_init();
  isr_init();
  hitLedTimer_init();
  trigger_init();
  trigger_enable();	// Makes the trigger state machine responsive to the trigger.
  // Init all interrupts (but does not enable the interrupts at the devices).
  // Prints an error message if an internal failure occurs because the argument = true.
  interrupts_initAll(true);
  interrupts_enableTimerGlobalInts();	// Allows the timer to generate interrupts.
  interrupts_startArmPrivateTimer();	// Start the private ARM timer running.
  interrupts_enableSysMonGlobalInts();	// Enable global interrupt of System Monitor.

  printf("Finished init for shooter mode\n");
  uint16_t histogramSystemTicks = 0;	// Only update the histogram display every so many ticks.
  double normalizedPowerValues[FILTER_IIR_FILTER_COUNT];// Use this to store normalized power values for the histogram.
  uint16_t indexOfMaxValue;	// Keep track of the index of the maximum value.
  intervalTimer_reset(0);	// Used to measure ISR execution time.
  intervalTimer_reset(1);	// Used to measure total program execution time.
  intervalTimer_reset(2);	// Used to measure main-loop execution time.
  intervalTimer_start(1);	// Start measuring total execution time.
  interrupts_enableArmInts();	// The ARM will start seeing interrupts after this.
  lockoutTimer_start();				   // Ignore erroneous hits at startup (when all power values are essentially 0).
  while (!(buttons_read() & BUTTONS_BTN3_MASK)) {  // Run until you detect btn3 pressed.
	  if (interrupts_isrFlagGlobal) {	           // Only do something if an interrupt has occurred.
		  intervalTimer_start(2);			   // Measure run-time when you are doing something.
		  histogramSystemTicks++;			   // Keep track of ticks so you know when to update the histogram.
		  countInterruptsViaInterruptsIsrFlag++;	   // Keep track of the interrupt-count based on the global flag.
		  interrupts_isrFlagGlobal = 0;		   // Reset the global flag.
		  detector();				   // Power across all channels is computed, hit-detection, etc.
      if (detector_hitDetected()) {		   // Hit detected?
		detector_clearHit();			   // Clear the hit.
		detector_hitCount_t hitCounts[DETECTOR_HIT_ARRAY_SIZE];	// Store the hit-counts here.
		detector_getHitCounts(hitCounts);			// Get the current hit counts.
		filter_getNormalizedPowerValues(normalizedPowerValues, &indexOfMaxValue);  // This normalizes power between 1 and 0.
		// Have the bar value and the label, send the data to the histogram.
		double normalizedHitValues[FILTER_IIR_FILTER_COUNT];  // Store normalized values here for the histogram.
		computeNormalizedHitValues(normalizedHitValues, hitCounts);  // Get the normalized hit values.
		for (int i=0; i<FILTER_IIR_FILTER_COUNT; i++) {		     // Iterate through the results for each channel.
		  char label[HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS];    // Get a buffer for the label.
		  // Create the label, based upon the actual power value.
		  if (snprintf(label, HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS, "%d", hitCounts[i]) == -1)
			printf("Error: snprintf encountered an error during conversion.\n\r");
		  histogram_setBarData(i, normalizedHitValues[i] * HISTOGRAM_MAX_BAR_DATA_IN_PIXELS, label);
		  histogram_updateDisplay();	// Redraw the histogram.
		}
      }
      uint16_t switchValue = switches_read();	// Read the switches and switch frequency as required.
      // Note that Brian sends the coefficients with the min. frequency at 0, max. frequency at 9. Transmitter does likewise.
      transmitter_setFrequencyNumber(switchValue);
    }
    intervalTimer_stop(2);  // All done with actual processing.
  }
  interrupts_disableArmInts();	// Done with loop, disable the interrupts.
  printRunTimeStatistics();    // Print the statistics to the TFT.
}

static double detectorTestValues[500] = 
{
  1181,
  1421,
  1518,
  1394,
  1223,
  1305,
  1300,
  1100,
  1157,
  1054,
  1436,
  1137,
  1134,
  1305,
  1066,
  1059,
  1219,
  1372,
  1037,
  1266,
  1102,
  1127,
  977,
  1387,
  1496,
  1029,
  1261,
  1337,
  1326,
  1346,
  1314,
  1170,
  1224,
  1099,
  1544,
  1144,
  1071,
  1276,
  1402,
  1204,
  1270,
  1238,
  1066,
  1325,
  1076,
  1136,
  1262,
  1215,
  1275,
  1287,
  1088,
  1006,
  1422,
  1308,
  1201,
  1443,
  966,
  1254,
  1244,
  1507,
  1283,
  1364,
  1494,
  1069,
  945,
  1236,
  1183,
  1223,
  1177,
  986,
  1258,
  1176,
  1337,
  1376,
  1308,
  1045,
  1098,
  1350,
  1017,
  1176,
  1120,
  1123,
  1116,
  1161,
  1313,
  1138,
  897,
  989,
  1422,
  1332,
  1199,
  1305,
  1356,
  1202,
  1309,
  1268,
  1261,
  1274,
  1051,
  1310,
  1023,
  1109,
  1164,
  1281,
  1356,
  1231,
  1073,
  1207,
  1373,
  1156,
  1243,
  1453,
  1208,
  1451,
  1313,
  1249,
  1183,
  1397,
  1269,
  1043,
  1232,
  1230,
  1252,
  1386,
  1480,
  1303,
  1419,
  1084,
  1343,
  1318,
  1361,
  1358,
  1025,
  1277,
  1350,
  1049,
  1195,
  1133,
  1106,
  1371,
  953,
  1129,
  1300,
  1395,
  1520,
  1220,
  1335,
  1301,
  1113,
  1400,
  1242,
  1395,
  1111,
  1287,
  1240,
  1528,
  1422,
  1208,
  1009,
  1315,
  1261,
  1456,
  941,
  1327,
  1149,
  1345,
  1064,
  1129,
  1173,
  1259,
  1535,
  1285,
  1313,
  1357,
  1074,
  1200,
  1181,
  1104,
  1409,
  1295,
  1450,
  1388,
  1235,
  1397,
  1305,
  1724,
  1310,
  1307,
  1153,
  1111,
  1378,
  1124,
  1205,
  999,
  970,
  1349,
  1307,
  1147,
  1381,
  1180,
  1274,
  1218,
  1401,
  1320,
  1148,
  1249,
  1411,
  1210,
  1216,
  1090,
  1400,
  1210,
  1243,
  1113,
  1291,
  1182,
  1275,
  1208,
  1354,
  1197,
  1280,
  989,
  1248,
  1500,
  1147,
  1312,
  1301,
  1095,
  1202,
  1117,
  1105,
  1171,
  1448,
  1582,
  1165,
  1331,
  1436,
  1221,
  1473,
  1297,
  1380,
  1214,
  1170,
  1401,
  1281,
  1409,
  1296,
  1150,
  1182,
  1330,
  1181,
  1074,
  981,
  973,
  1159,
  1233,
  1201,
  1072,
  1202,
  1321,
  1145,
  1104,
  1312,
  1101,
  1321,
  1177,
  985,
  1329,
  1426,
  1124,
  1283,
  1268,
  1267,
  1430,
  1348,
  1260,
  1196,
  1202,
  1195,
  1187,
  1212,
  874,
  1185,
  1345,
  1018,
  1481,
  1248,
  1213,
  1093,
  1157,
  1114,
  1046,
  1160,
  1341,
  1299,
  1043,
  1239,
  1545,
  1250,
  1135,
  1193,
  890,
  1208,
  1127,
  1049,
  1249,
  1333,
  1253,
  1222,
  1039,
  1072,
  1255,
  1225,
  1175,
  1124,
  1277,
  1368,
  1092,
  1532,
  1147,
  1163,
  1199,
  1448,
  1168,
  1351,
  1179,
  1294,
  1392,
  1314,
  1129,
  1264,
  1318,
  1528,
  1288,
  1344,
  1175,
  1226,
  1045,
  1229,
  1073,
  1226,
  1229,
  1258,
  1555,
  1155,
  1230,
  1255,
  1160,
  1388,
  1203,
  1301,
  1073,
  1192,
  1096,
  1287,
  1438,
  1205,
  1213,
  1514,
  1360,
  1440,
  1366,
  1635,
  1346,
  1283,
  1302,
  1127,
  1298,
  1225,
  1320,
  1007,
  1193,
  1462,
  981,
  1140,
  1351,
  1223,
  1186,
  1141,
  1217,
  1486,
  1347,
  1153,
  1191,
  1359,
  1003,
  1115,
  1078,
  1061,
  1209,
  1322,
  1250,
  1266,
  1125,
  1160,
  1253,
  1495,
  1235,
  1268,
  1098,
  1332,
  1149,
  1012,
  1407,
  1365,
  1305,
  1045,
  1076,
  1351,
  1107,
  1086,
  1292,
  1003,
  1366,
  1319,
  1391,
  1200,
  1270,
  1335,
  1275,
  1250,
  1394,
  1190,
  1197,
  1206,
  1265,
  1472,
  1485,
  1332,
  1230,
  1335,
  1066,
  1251,
  1281,
  976,
  1312,
  1037,
  1331,
  1135,
  1210,
  1076,
  1093,
  1078,
  1151,
  1279,
  1045,
  1278,
  1138,
  1415,
  1313,
  1349,
  1207,
  1023,
  1179,
  1170,
  1351,
  1277,
  1361,
  957,
  1503,
  1253,
  1389,
  1049,
  1216,
  925,
  1183,
  1272,
  1292,
  1212,
  1216,
  1088,
  1307,
  1351,
  1415,
  1413,
  1405,
  1101,
  1063,
  1306,
  1321,
  1016,
  1034,
  1393,
  1338,
  1180,
  1355,
  1213,
  1320,
  1350,
  1201,
  1159,
  1079,
  990,
  1198,
  1073,
  1098,
  1117,
  1220,
  1030,
  1236
};



// Default is continuous-power mode. Hold btn2 during reset/power-up to come up in shooter mode.
int main() {
  buttons_init();
  switches_init();
  double testVecTrue[10];
  double testVecFalse[10];
  double goldenMean1;
  double goldenMean2;

  shooterMode();

  uint16_t MEDIAN_INDEX = 4;

  for(int i=0; i<10; i++) {
      testVecTrue[i] = 50*(i+1);
      if(i==MEDIAN_INDEX)
        testVecTrue[i] = 2.4;
  }

  for(int i=0; i<10; i++) {
      testVecFalse[i] = 50*(i+1);
  }

  goldenMean1 = testVecTrue[MEDIAN_INDEX];
  goldenMean2 = testVecFalse[MEDIAN_INDEX];

  printf("%lf\n", detectorTestValues[459]);

  detector_runTest(testVecTrue, testVecFalse, goldenMean1, goldenMean2);
//  filter_runTest();
  printf("beginning test!\n");
  // continuousPowerMode();
  shooterMode();
  printf("Done!\n");
}

