/*
* Author: Sebastian Boettcher
* 
* Wrapper class for the Sharp Display using GrLib and LcdDriver code.
*
*/

#ifndef display_edison_h
#define display_edison_h

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

#include <math.h>
#include <time.h>

#include <pthread.h>

#include "Sharp96x96.h"
#include "Sharp128x128.h"
#include "LcdDriver.h"
#include "grlib.h"

#define PI 3.14159265

class display_edison {
 public:
  // standard constructor, initializes the display with a black on white 6x8 font
  display_edison(uint8_t res = 128, uint8_t clk_hands=3);
  // destructor, closes mraa ports
  ~display_edison();

  // init display
  void init();
  // stop display
  void stop();

  // prints string s to the display at the given position
  void print(std::string s, int x, int y, bool centered = false, bool opaque = true);
  // prints float f to the display at the given position and with given precision
  void print(float f, int x, int y, int precision = 5, bool centered = false, bool opaque = true);
  // prints integer i to the display at the given position
  void print(int i, int x, int y, bool centered = false, bool opaque = true);

  // prints the current battery charge in the display corner
  void batteryCharge(int charge);

  // prints the current time as an analog clock to the display
  void analogClock(bool force_refresh = false);
  // prints the specified time as an analog clock to the display
  void analogClock(struct tm * timeinfo, bool force_refresh = false);

  // flushes the display buffer
  void flush();

  // clears the display and resets the display buffer
  void clear();

  bool is_refreshed() {return m_refreshed;}
  bool is_active() {return m_active;}

  uint8_t resolution() {return m_res;}

  // return pointer to internal graphics context
  Graphics_Context* context() {return &g_sContext;}

  // spawn and join display refresh thread
  // according to the display data sheet the internal frame buffer needs
  // to be refreshed at ca. 60Hz during operation
  void startThread();
  void stopThread();

 private:
  void t_dspRefresh();

  int ccharge; // charge cache 
  int chour, cminute, csecond; // time cache 
  uint8_t m_res;
  uint8_t c_hands;
  tContext g_sContext;
  bool m_refreshed;
  bool m_active;

  bool m_threaded;
  bool m_flush; // flag to trigger flush during threaded operation
  bool m_clear; // flag to trigger clear during threaded operation
  std::thread m_thread_dspRefresh;
};

#endif // display_edison_h

