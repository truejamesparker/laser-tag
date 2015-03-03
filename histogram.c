#include "histogram.h"
#include "supportFiles/display.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "supportFiles/utils.h"
#include <string.h>

#define TOP_LABEL_TEXT_SIZE 1

static uint16_t histogram_barWidth;				// May share this with other functions in this package.
static uint16_t topLabelMaxWidthInChars;	// How many chars will be printed.
static histogram_data_t currentBarData[HISTOGRAM_BAR_COUNT];	// Current histogram data.
static histogram_data_t previousBarData[HISTOGRAM_BAR_COUNT];	// Keep the old stuff around so you can erase things properly.
static char topLabel[HISTOGRAM_BAR_COUNT][HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS];		// Labels at top of histogram bars.
static char oldTopLabel[HISTOGRAM_BAR_COUNT][HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS];	// Old label so you only update as necessary.

static bool initFlag = false;	// Keep track whether histogram_init() has been called.
// These are the default colors for the bars.
static uint16_t histogram_barColors[HISTOGRAM_MAX_BAR_COUNT] = {DISPLAY_BLUE, DISPLAY_RED, DISPLAY_GREEN, DISPLAY_CYAN, DISPLAY_MAGENTA,
								DISPLAY_YELLOW, DISPLAY_WHITE, DISPLAY_BLUE, DISPLAY_RED, DISPLAY_GREEN};
// Default colors for the white dynamic labels.
static uint16_t histogram_barTopLabelColors[HISTOGRAM_MAX_BAR_COUNT] = {DISPLAY_WHITE, DISPLAY_WHITE, DISPLAY_WHITE, DISPLAY_WHITE, DISPLAY_WHITE,
									DISPLAY_WHITE, DISPLAY_WHITE, DISPLAY_WHITE, DISPLAY_WHITE, DISPLAY_WHITE};
// Default labels for the histogram bars.
// These labels do not change during operation.
static char histogram_label[HISTOGRAM_MAX_BAR_COUNT][HISTOGRAM_MAX_BAR_LABEL_WIDTH] = {{"0"}, {"1"}, {"2"},{"3"}, {"4"}, {"5"}, {"6"}, {"7"}, {"8"}, {"9"}};

// The bottom labels are drawn at the bottom of the bar and are static.
void histogram_drawBottomLabels() {
  uint16_t labelOffset = (histogram_barWidth - (DISPLAY_CHAR_WIDTH * HISTOGRAM_BOTTOM_LABEL_TEXT_SIZE))/2;	// Center the label.
  display_setTextSize(HISTOGRAM_BOTTOM_LABEL_TEXT_SIZE);	// Set the text-size.
  for (int i=0; i<HISTOGRAM_BAR_COUNT; i++) {		//
    display_setCursor(i*(histogram_barWidth+HISTOGRAM_BAR_X_GAP) + labelOffset, display_height()-(DISPLAY_CHAR_HEIGHT * HISTOGRAM_BOTTOM_LABEL_TEXT_SIZE)+2);
    display_setTextColor(histogram_barColors[i]);
    display_print(histogram_label[i]);
  }
}

void histogram_init() {
  if (HISTOGRAM_BAR_COUNT > HISTOGRAM_MAX_BAR_COUNT) {
    printf("Error: histogram_init(): HISTOGRAM_BAR_COUNT (%d) is larger than the max allowed (%d)", HISTOGRAM_BAR_COUNT, HISTOGRAM_MAX_BAR_COUNT);
    exit(0);
  }
  if (initFlag) return;  // If already called, do nothing.
  display_init();	 // Init the display package.
  histogram_barWidth = (display_width() / HISTOGRAM_BAR_COUNT) - HISTOGRAM_BAR_X_GAP;
  topLabelMaxWidthInChars = (histogram_barWidth/DISPLAY_CHAR_WIDTH);   // The top-label can be this wide.
  // But, double-check to make sure that it will fit in the memory allocated for the label array. Set to fit allocated area in any case.
  // The -1 allows space for the 0 that ends the string.
  topLabelMaxWidthInChars = (topLabelMaxWidthInChars < (HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS - 1)) ?
    topLabelMaxWidthInChars : HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS - 1;
  for (int i=0; i<HISTOGRAM_BAR_COUNT; i++) {
    currentBarData[i] = 0;
    previousBarData[i] = 0;
    topLabel[i][0] = 0;			// Start out with empty strings.
    oldTopLabel[i][0] = 0;	// Start out with empty strings.
  }
  display_fillScreen(DISPLAY_BLACK);
  histogram_drawBottomLabels();
  initFlag = true;
}


// This function only updates the data for the histogram. histogram_updateDisplay() will do the actual drawing.
// barIndex is the index of the histogram bar, 0 is left, larger indices to the right.
// data is the value of the bar in PIXELS.
// barTopLabel is a dynamic label that will be drawn at the top of the bar. It will be redrawn whenever
// a change is detected.
bool histogram_setBarData(histogram_index_t barIndex, histogram_data_t data, const char barTopLabel[]) {
  if (!initFlag) {
    printf("Error! histogram_setBarData(): must call histogram_init() before calling this function.\n\r");
    return false;
  }
  // Error checking.
  if (barIndex > HISTOGRAM_BAR_COUNT) {
    printf("Error! histogram_setBarData(): barIndex(%d) is greater than maximum (%d)\n\r", barIndex, HISTOGRAM_BAR_COUNT-1);
    return false;
  }
  // Error checking.
  if (data > HISTOGRAM_MAX_BAR_DATA_IN_PIXELS) {
    printf("Error! histogram_setBarData(): data (%d) is greater than maximum (%d) for index(%d) \n\r", data, HISTOGRAM_MAX_BAR_DATA_IN_PIXELS-1, barIndex);
    return false;
  }
  // If it has changed, update the data in the array but don't render anything on the display.
  if (data != currentBarData[barIndex]) {													// Only modify the data if it has changed.
    previousBarData[barIndex] = currentBarData[barIndex];  // Save the old data so you render things properly.
    // Copy the old string so that you can render things properly.
    currentBarData[barIndex] = data;															// Store the data because it changed.
  }
  // If the label has changed, move the previous data to old, new data to current.
  // Labels are handled separately from data because the label may change even if the underlying bar data does not.
  // This allows the top label to change and to be redrawn even if the bars stay the same height.
  if (strncmp(barTopLabel, topLabel[barIndex], HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS)) {
    // If you get here, the new label is different from the last one.
    strncpy(oldTopLabel[barIndex], topLabel[barIndex], HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS);
    // Copy the current label to become the new old label.
    strncpy(topLabel[barIndex], barTopLabel, HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS);
    // Copy the new label to become the current label.
    uint16_t barTopLabelLength = strlen(barTopLabel);									// Get the length of the label.
    // Only copy as many characters as will fit in the available screen space.
    uint16_t charCopyLimit = (barTopLabelLength < topLabelMaxWidthInChars) ? barTopLabelLength : topLabelMaxWidthInChars;
    for (uint16_t i=0; i<charCopyLimit; i++)
      topLabel[barIndex][i] = barTopLabel[i];
    // Null terminate the string in any case.
    topLabel[barIndex][charCopyLimit] = 0;
  }
  return true;  // Everything is OK.
}

// Internal helper function.
// Erases the old text (using a fillRect because it is small and fast) to erase the old label, if required.
// Finds the position for the label, just above the top of the bar.
void histogram_drawTopLabel(uint16_t barIndex, histogram_data_t data, const char topLabel[], bool eraseOldLabel) {
  if (eraseOldLabel) {
    // Erase with a fillRect because the rect is small and should be faster than hitting individual label pixels.
    display_fillRect(barIndex*(histogram_barWidth+HISTOGRAM_BAR_X_GAP),
		     display_height() - data - HISTOGRAM_BAR_Y_GAP - DISPLAY_CHAR_HEIGHT - 1,
		     histogram_barWidth, DISPLAY_CHAR_HEIGHT, DISPLAY_BLACK);

  }
  uint16_t topLabelXOffset = (histogram_barWidth - (strlen(topLabel)*DISPLAY_CHAR_WIDTH)) / 2;	// This helps to center the label over the bar.
  display_setCursor(barIndex*(histogram_barWidth+HISTOGRAM_BAR_X_GAP) + topLabelXOffset,				// This is the location of the top label.
		    display_height() - data - HISTOGRAM_BAR_Y_GAP - DISPLAY_CHAR_HEIGHT - 1);
  display_setTextSize(TOP_LABEL_TEXT_SIZE);																																				// Use tiny text to pack more characters into the label.
  display_setTextColor(histogram_barTopLabelColors[barIndex]);		// Set the color of the label.
  display_print(topLabel);																				// Draw the label.
}

// This updates the display.
// It loops across all bars, checking:
// If the height of the bar has changed, redraw both the bar and the top label.
// If the height of the bar has not changed, but the top label has changed, update the label.
void histogram_updateDisplay() {
  if (!initFlag) {
    printf("Error! histogram_displayUpdate(): must call histogram_init() before calling this function.\n\r");
    return;
  }
  for (int i=0; i<HISTOGRAM_BAR_COUNT; i++) {
    histogram_data_t oldData = previousBarData[i];	// Get the previous data.
    histogram_data_t data = currentBarData[i];		// Get the current bar data.
    if (oldData != data) {  // If the are not equal, redraw the bar and the top-label.
      // Erase the old bar and extend the erase rectangle to include the top-label so that everything is erased at once.
      // Also, redraw the top label.
      display_fillRect(i*(histogram_barWidth+HISTOGRAM_BAR_X_GAP), display_height() - oldData - HISTOGRAM_BAR_Y_GAP - DISPLAY_CHAR_HEIGHT - 1,
		       histogram_barWidth, oldData + DISPLAY_CHAR_HEIGHT + 1, DISPLAY_BLACK);
      // Draw the new bar.
      display_fillRect(i*(histogram_barWidth+HISTOGRAM_BAR_X_GAP), display_height() - data - HISTOGRAM_BAR_Y_GAP,
		       histogram_barWidth, data, histogram_barColors[i]);
      if (data != 0) {	// Only draw the top label if the bar-data != 0.
	histogram_drawTopLabel(i, data, topLabel[i], false);  // false means that the old label does not need to be erased.
	previousBarData[i] = currentBarData[i];	// Old data and new data are the same after the update.
	// Old label and new label are the same after the update.
	strncpy(oldTopLabel[i], topLabel[i], HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS);
      }
    } else if ((data != 0) && strncmp(topLabel[i], oldTopLabel[i], HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS)) {
      histogram_drawTopLabel(i, data, topLabel[i], true);  // True means that the old label needs to be erased.
      // After the update, copy the label to old data so that it won't reupdate until the next change.
      strncpy(oldTopLabel[i], topLabel[i], HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS);
    }
  }
}

// Runs a short test that writes random values to the histogram bar-values as
// specified by the #defines below.
#define RANDOM_LABEL "99"
#define HISTOGRAM_RUN_TEST_ITERATION_COUNT 10
#define HISTOGRAM_RUN_TEST_LOOP_DELAY_MS 500
void histogram_runTest() {
  histogram_init();	// Must init the histogram data structures.
  for (int i=0; i<HISTOGRAM_RUN_TEST_ITERATION_COUNT; i++) {	// Loop as required.
    for (int j=0; j<HISTOGRAM_BAR_COUNT; j++) {			// set each of the bar values.
      histogram_setBarData(j, rand() % HISTOGRAM_MAX_BAR_DATA_IN_PIXELS, RANDOM_LABEL);	// set the bar data to a random value.
    }
    histogram_updateDisplay();	// update the display.
    utils_msDelay(HISTOGRAM_RUN_TEST_LOOP_DELAY_MS);  // Slow the update so you can see it happen.
  }
}