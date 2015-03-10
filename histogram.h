#ifndef HISTOGRAM_H_
#define HISTOGRAM_H_

#include "supportFiles/display.h"

#include <stdint.h>

#define HISTOGRAM_BOTTOM_LABEL_TEXT_SIZE 2	// Text-size for the bottom label.

// Allow up to this many chars for the label on top of the histogram bar. Actually printed chars depends upon width of bar.
#define HISTOGRAM_BAR_TOP_MAX_LABEL_WIDTH_IN_CHARS	32
#define HISTOGRAM_TOP_LABEL_HEIGHT 20	// Allow some room for a label above each bar (in pixels)

#define HISTOGRAM_MAX_BAR_COUNT 10		// You can have up to 10 bars on your histogram.
#define HISTOGRAM_BAR_COUNT 10				// This is the number of histogram bars that you want.
#define HISTOGRAM_BAR_X_GAP 5					// This is the gap, in pixels, between each bar.
#define HISTOGRAM_BAR_Y_GAP (DISPLAY_CHAR_HEIGHT * HISTOGRAM_BOTTOM_LABEL_TEXT_SIZE)	// Leave room for a small label.
#define HISTOGRAM_MAX_BAR_DATA_IN_PIXELS (DISPLAY_HEIGHT - HISTOGRAM_BAR_Y_GAP -  HISTOGRAM_TOP_LABEL_HEIGHT)	// Max value (height) for histogram bar, in pixels.
#define HISTOGRAM_MAX_BAR_LABEL_WIDTH 2	// Defined in terms of characters.

#define DISPLAY_CHAR_WIDTH 6
#define DISPLAY_CHAR_HEIGHT 8

#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240

typedef uint16_t histogram_index_t;	// Used to index each histogram bar.
typedef uint16_t histogram_data_t;	// The data associated with each bar.

// Must call this before using the histogram functions.
void histogram_init();

// Sets the height (data) of the bar (barIndex).
// Also places a small label (barTopLabel) at the top of the bar. Does NOT render the histogram onto the TFT.
// Returns false if there is something wrong with the provided arguments.
bool histogram_setBarData(histogram_index_t barIndex, histogram_data_t data, const char barTopLabel[]);

// Call this to draw the histogram with the data from histogram_setBarData().
void histogram_updateDisplay();

// Runs a simple test.
void histogram_runTest();

#endif /* HISTOGRAM_H_ */