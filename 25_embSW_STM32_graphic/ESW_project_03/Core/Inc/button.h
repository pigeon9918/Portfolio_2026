#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdint.h>

// 24x24 cursor dimensions
#define CURSOR_24x24_WIDTH  24
#define CURSOR_24x24_HEIGHT 24

// Initial position for the 24x24 cursor
#define CURSOR_START_X      400
#define CURSOR_START_Y      400

// --- Button Icon Data Declarations ---
// The actual definitions are in button.c
// 32x32 pixels, 1-bit monochrome. Each icon is 128 bytes.
extern const uint16_t cursor_icon_24x24[24 * 24];
extern const uint16_t button_power_icon_32x32[32 * 32];


// --- Button Layout Definitions ---
// Coordinates are based on an assumed 320x240 display.
#define BUTTON_WIDTH    32
#define BUTTON_HEIGHT   32

// POWER Button (toggles animation On/Off)
#define BUTTON_POWER_X      10
#define BUTTON_POWER_Y      200

// COLOR CHANGE Button
#define BUTTON_COLOR_X      320 - 32 - 10
#define BUTTON_COLOR_Y      200

// --- Button Identification Enum ---
typedef enum {
    BUTTON_NONE = 0,
    BUTTON_POWER,
    BUTTON_COLOR_CHANGE,
    BUTTON_COUNT
} ButtonId_t;

#endif // BUTTON_H_
