#include "transmitter.h"
#include "supportFiles/intervalTimer.h"
#include "trigger.h"
#include "filter.h"
#include "hitLedTimer.h"
#include "lockoutTimer.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "supportFiles/mio.h"
#include "supportFiles/utils.h"
#include "supportFiles/interrupts.h"
#include "supportFiles/buttons.h"
#include "supportFiles/switches.h"
#include "supportFiles/leds.h"
#include "isr.h"
// #include "f_loader.h"


static double inputSignal[300] = 
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
1135
};
 
int main() {
	int end = 300;
	filter_init();
	filter_runPowerTest(inputSignal, end);
	// // file_loadDataToArray(inputSignal, end, "inputSignal.txt");
	// printf("%lf\n", inputSignal[0]);
	// printf("%lf\n", inputSignal[end-1]);

	// filter_init();
	// double normalizedArray[10];
	// uint16_t winner;
	// uint16_t last_winner=11;
	// int count = 0;
	// int player = 0;
	// uint32_t j = 0;
	// for (int j=0; j<end; j++) {
	// 	filter_addNewInput(inputSignal[j]);	
	// 	if (count==10) {
	// 		filter_firFilter();
	// 		for (int i=0; i<10; i++) {
	// 			filter_iirFilter(i);
	// 			filter_computePower(i, false, false);
	// 			// printf("Current Power: %lf\n", filter_getCurrentPowerValue(i));
	// 		}

	// 		// filter_getNormalizedPowerValues(normalizedArray, &winner);
	// 		// if (winner!=last_winner) {
	// 		// 	printf("Top Power: %u\n", winner);
	// 		// 	last_winner = winner;
	// 		// }
	// 		count = 0;
	// 	}
	// 	count++;
	// }
// }
	
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

	lockoutTimer_runTest();
	hitLedTimer_runTest();
	transmitter_runTest();
	while(1) {
		transmitter_setFrequencyNumber(switches_read());
	}
}

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
// }*/
