/*
* Author: Sebastian Boettcher
* 
* Wrapper class for the Sharp 96x96 Display using GrLib and LcdDriver code.
*
*/

#include "./display_edison.h"


//_______________________________________________________________________________________________________
display_edison::display_edison(uint8_t clk_hands) : c_hands(clk_hands) {
  HAL_LCD_initDisplay();
  Graphics_initContext(&g_sContext, &g_sharp96x96LCD);
  Graphics_setForegroundColor(&g_sContext, ClrWhite);
  Graphics_setBackgroundColor(&g_sContext, ClrBlack);
  Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
  Graphics_clearDisplay(&g_sContext);
  Graphics_flushBuffer(&g_sContext);
  
  printf("[DSP] Initialized.\n");
  fflush(stdout);
}


//_______________________________________________________________________________________________________
display_edison::~display_edison() {
  Display_Stop();

  printf("[DSP] Stopped.\n");
  fflush(stdout);
}


//_______________________________________________________________________________________________________
void display_edison::print(std::string s, int x, int y, bool centered, bool opaque) {
  if (centered) {
    if (opaque)
      Graphics_drawStringCentered(&g_sContext, (char *) s.c_str(), AUTO_STRING_LENGTH, x, y, OPAQUE_TEXT);
    else
      Graphics_drawStringCentered(&g_sContext, (char *) s.c_str(), AUTO_STRING_LENGTH, x, y, TRANSPARENT_TEXT);
  } else {
    if (opaque)
      Graphics_drawString(&g_sContext, (char *) s.c_str(), AUTO_STRING_LENGTH, x, y, OPAQUE_TEXT);
    else
      Graphics_drawString(&g_sContext, (char *) s.c_str(), AUTO_STRING_LENGTH, x, y, TRANSPARENT_TEXT);
  }
}

//_______________________________________________________________________________________________________
void display_edison::print(float f, int x, int y, int precision, bool centered, bool opaque) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(precision) << f;
  if (centered) {
    if (opaque)
      Graphics_drawStringCentered(&g_sContext, (char *) ss.str().c_str(), AUTO_STRING_LENGTH, x, y, OPAQUE_TEXT);
    else
      Graphics_drawStringCentered(&g_sContext, (char *) ss.str().c_str(), AUTO_STRING_LENGTH, x, y, TRANSPARENT_TEXT);
  } else {
    if (opaque)
      Graphics_drawString(&g_sContext, (char *) ss.str().c_str(), AUTO_STRING_LENGTH, x, y, OPAQUE_TEXT);
    else
      Graphics_drawString(&g_sContext, (char *) ss.str().c_str(), AUTO_STRING_LENGTH, x, y, TRANSPARENT_TEXT);
  }
}

//_______________________________________________________________________________________________________
void display_edison::print(int i, int x, int y, bool centered, bool opaque) {
   if (centered) {
    if (opaque)
      Graphics_drawStringCentered(&g_sContext, (char *) std::to_string(i).c_str(), AUTO_STRING_LENGTH, x, y, OPAQUE_TEXT);
    else
      Graphics_drawStringCentered(&g_sContext, (char *) std::to_string(i).c_str(), AUTO_STRING_LENGTH, x, y, TRANSPARENT_TEXT);
  } else {
    if (opaque)
      Graphics_drawString(&g_sContext, (char *) std::to_string(i).c_str(), AUTO_STRING_LENGTH, x, y, OPAQUE_TEXT);
    else
      Graphics_drawString(&g_sContext, (char *) std::to_string(i).c_str(), AUTO_STRING_LENGTH, x, y, TRANSPARENT_TEXT);
  }
}

//_______________________________________________________________________________________________________
void display_edison::batteryCharge(int charge) {
  if (ccharge == charge)
    return; // no need to update
  std::ostringstream bstr;
    bstr << std::setw(3) << std::setfill(' ') << charge;
      std::string batstr = bstr.str();
  if (LCD_VERTICAL_MAX==96) {
    Graphics_drawLine(&g_sContext,90,9,92,9);  
    Graphics_drawLine(&g_sContext,88,10,94,10);
    Graphics_drawLine(&g_sContext,88,11,88,19);
    Graphics_drawLine(&g_sContext,94,11,94,20);
    for (int i=0; i<(0.1*charge); i++)  
      Graphics_drawLine(&g_sContext,89,20-i,93,20-i);  
    print(batstr, 87, 3, true);
  }
  else if (LCD_VERTICAL_MAX==128) {
    Graphics_drawLine(&g_sContext,120,9,124,9);  
    Graphics_drawLine(&g_sContext,120,10,124,10);
    Graphics_drawLine(&g_sContext,118,11,126,11);
    Graphics_drawLine(&g_sContext,118,12,118,25);
    Graphics_drawLine(&g_sContext,126,12,126,25);
    for (int i=0; i<(0.15*charge); i++)
      Graphics_drawLine(&g_sContext,119,25-i,125,25-i);
    print(batstr, 120, 3, true);
  }  
}


//_______________________________________________________________________________________________________
void display_edison::analogClock(bool force_refresh) {
  time_t rawtime;
  time(&rawtime);
  analogClock(localtime(&rawtime), force_refresh);
}
//_______________________________________________________________________________________________________
void display_edison::analogClock(struct tm * timeinfo, bool force_refresh) {
 
  int hour = timeinfo->tm_hour;
  int minute = timeinfo->tm_min;
  int second = timeinfo->tm_sec;

  if (!force_refresh && ((c_hands<3) && chour == hour) && (cminute == minute)) {
	  m_refreshed = false;
	  return; // no need to update
  } else if (!force_refresh && (c_hands>2) && (cminute == minute) && (csecond == second)) {
	  m_refreshed = false;
	  return; // no need to update	
  } else {
	  m_refreshed = true;
  }

  chour = hour;
  cminute = minute;

  int cX = (LCD_HORIZONTAL_MAX/2); // center X
  int cY = (LCD_VERTICAL_MAX/2); // center Y
  int r = (LCD_HORIZONTAL_MAX-1)/2; // radius
  int margin = 2; // margin from outer circle

  // clear display
  clear();

  // draw static part of clock face
  Graphics_drawCircle(&g_sContext, cX, cY, r); // outer circle
  Graphics_drawCircle(&g_sContext, cX, cY, r+1); // outer circle
  Graphics_drawCircle(&g_sContext, cX, cY, r+2); // outer circle

  // ticks
  for (int i = 1; i <= 60; ++i) {
    float angle = -PI/2.0+((i/60.0)*2.0*PI);
    int x1 = cX + cos(angle) * (r - margin);
    int y1 = cY + sin(angle) * (r - margin);
    if (i % 15 == 0) { // major ticks, every 15 minutres
      int x2 = cX + cos(angle) * (r - margin - 6);
      int y2 = cY + sin(angle) * (r - margin - 6);
      Graphics_drawLine(&g_sContext, x1, y1, x2, y2);
    } else if (i % 5 == 0) { // minor ticks, every 5 minutes
      int x2 = cX + cos(angle) * (r - margin - 4);
      int y2 = cY + sin(angle) * (r - margin - 4);
      Graphics_drawLine(&g_sContext, x1, y1, x2, y2);
    } else { // single second ticks
      //int x2 = cX + cos(angle) * (r - margin - 1);
      //int y2 = cY + sin(angle) * (r - margin - 1);
      //Graphics_drawLine(&g_sContext, x1, y1, x2, y2);
    }
  }

  // draw time
  std::ostringstream hstr, mstr;
  hstr << std::setw(2) << std::setfill('0') << hour;
  mstr << std::setw(2) << std::setfill('0') << minute;
  std::string timestr = hstr.str() + ":" + mstr.str();
  print(timestr, cY, 25, true);
  
  // draw clock hands
  int x1 = cX;
  int y1 = cY;
  int x2 = 0;
  int y2 = 0;
  float angle = 0.0;
  int hour_len = (LCD_VERTICAL_MAX==96)?20:30;
  int min_len  = (LCD_VERTICAL_MAX==96)?30:45;

  // hour (+ interpolation based on current minute)
  angle = -PI/2.0 + ((hour / 12.0) * 2.0 * PI)+((minute / 60.0) * PI / 6.0);
  x2 = cX + cos(angle) * hour_len;
  y2 = cY + sin(angle) * hour_len;
  Graphics_drawLine(&g_sContext, x1-1, y1, x2, y2);
  Graphics_drawLine(&g_sContext, x1, y1-1, x2, y2);
  Graphics_drawLine(&g_sContext, x1+1, y1, x2, y2);
  Graphics_drawLine(&g_sContext, x1, y1+1, x2, y2);

  // draw minute hand
  if (c_hands>1) {
    angle = -PI/2.0 + ((minute / 60.0) * 2.0 * PI);
    x2 = cX + cos(angle) * min_len;
    y2 = cY + sin(angle) * min_len;
    Graphics_drawLine(&g_sContext, x1-1, y1, x2, y2);
    Graphics_drawLine(&g_sContext, x1, y1-1, x2, y2);
    Graphics_drawLine(&g_sContext, x1+1, y1, x2, y2);
    Graphics_drawLine(&g_sContext, x1, y1+1, x2, y2);
  }

  // draw second hand
  if (c_hands>2) { 
    csecond = second;
    angle = -PI/2.0 + ((second / 60.0) * 2.0 * PI);
    x2 = cX + cos(angle) * 35;
    y2 = cY + sin(angle) * 35;
    Graphics_drawLine(&g_sContext, x1, y1, x2, y2);
  }

}



//_______________________________________________________________________________________________________
void display_edison::flush() {
  Graphics_flushBuffer(&g_sContext);
}

//_______________________________________________________________________________________________________
void display_edison::clear() {
  Graphics_clearDisplay(&g_sContext);
}
