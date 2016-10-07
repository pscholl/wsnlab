#ifndef DISPLAY_MCU_H_
#define DISPLAY_MCU_H_

#include "Sharp96x96.h"
#include "Sharp128x128.h"
#include "LcdDriver.h"
#include "grlib.h"
#include <time.h>

tContext g_sContext;
uint8_t res;

// amount of hands
int c_hands;

// init display
void init_display(int clk_hands);

// clears the display and resets the display buffer
void clear_display();

// flushes the display buffer
void flush();

// prints string s to the display at the given position (default was: centered = false, opaque = true)
void print_string(char* s, int x, int y, _Bool centered, _Bool opaque);

// stop display
void stop_display();

// prints the specified time as an analog clock to the display
void analogClock(struct tm * timeinfo);

// prints the current battery charge in the display corner
void batteryCharge(int charge, _Bool charging);

// prints a warning icon in the left display corner
void warning(_Bool warn);

// returns 1 if the time or the battery charge has changed
_Bool DSPChanges(struct tm * timeinfo, int charge, _Bool charging);

#endif /* DISPLAY_MCU_H_ */
