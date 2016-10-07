#include "mcu_api.h"
#include "display_mcu.h"

int chour, cminute, csecond = 0; // time cache
int ccharge = 0; // bat charge cache
_Bool ccharging = 0; // bat charging cache

int sin10000angle[721] = {-10000, -9999, -9998, -9996, -9993, -9990, -9986, -9981, -9975, -9969, -9961, -9953, -9945, -9935, -9925, -9914, -9902, -9890, -9876, -9862, -9848,
		-9832, -9816, -9799, -9781, -9762, -9743, -9723, -9702, -9681, -9659, -9636, -9612, -9588, -9563, -9537, -9510, -9483, -9455, -9426, -9396,
		-9366, -9335, -9304, -9271, -9238, -9205, -9170, -9135, -9099, -9063, -9025, -8987, -8949, -8910, -8870, -8829, -8788, -8746, -8703, -8660,
		-8616, -8571, -8526, -8480, -8433, -8386, -8338, -8290, -8241, -8191, -8141, -8090, -8038, -7986, -7933, -7880, -7826, -7771, -7716, -7660,
		-7604, -7547, -7489, -7431, -7372, -7313, -7253, -7193, -7132, -7071, -7009, -6946, -6883, -6819, -6755, -6691, -6626, -6560, -6494, -6427,
		-6360, -6293, -6225, -6156, -6087, -6018, -5948, -5877, -5807, -5735, -5664, -5591, -5519, -5446, -5372, -5299, -5224, -5150, -5075, -5000,
		-4924, -4848, -4771, -4694, -4617, -4539, -4461, -4383, -4305, -4226, -4146, -4067, -3987, -3907, -3826, -3746, -3665, -3583, -3502, -3420,
		-3338, -3255, -3173, -3090, -3007, -2923, -2840, -2756, -2672, -2588, -2503, -2419, -2334, -2249, -2164, -2079, -1993, -1908, -1822, -1736,
		-1650, -1564, -1478, -1391, -1305, -1218, -1132, -1045, -958, -871, -784, -697, -610, -523, -436, -348, -261, -174, -87, 0,
		87, 174, 261, 348, 436, 523, 610, 697, 784, 871, 958, 1045, 1132, 1218, 1305, 1391, 1478, 1564, 1650, 1736,
		1822, 1908, 1993, 2079, 2164, 2249, 2334, 2419, 2503, 2588, 2672, 2756, 2840, 2923, 3007, 3090, 3173, 3255, 3338, 3420,
		3502, 3583, 3665, 3746, 3826, 3907, 3987, 4067, 4146, 4226, 4305, 4383, 4461, 4539, 4617, 4694, 4771, 4848, 4924, 4999,
		5075, 5150, 5224, 5299, 5372, 5446, 5519, 5591, 5664, 5735, 5807, 5877, 5948, 6018, 6087, 6156, 6225, 6293, 6360, 6427,
		6494, 6560, 6626, 6691, 6755, 6819, 6883, 6946, 7009, 7071, 7132, 7193, 7253, 7313, 7372, 7431, 7489, 7547, 7604, 7660,
		7716, 7771, 7826, 7880, 7933, 7986, 8038, 8090, 8141, 8191, 8241, 8290, 8338, 8386, 8433, 8480, 8526, 8571, 8616, 8660,
		8703, 8746, 8788, 8829, 8870, 8910, 8949, 8987, 9025, 9063, 9099, 9135, 9170, 9205, 9238, 9271, 9304, 9335, 9366, 9396,
		9426, 9455, 9483, 9510, 9537, 9563, 9588, 9612, 9636, 9659, 9681, 9702, 9723, 9743, 9762, 9781, 9799, 9816, 9832, 9848,
		9862, 9876, 9890, 9902, 9914, 9925, 9935, 9945, 9953, 9961, 9969, 9975, 9981, 9986, 9990, 9993, 9996, 9998, 9999, 10000,
		9999, 9998, 9996, 9993, 9990, 9986, 9981, 9975, 9969, 9961, 9953, 9945, 9935, 9925, 9914, 9902, 9890, 9876, 9862, 9848,
		9832, 9816, 9799, 9781, 9762, 9743, 9723, 9702, 9681, 9659, 9636, 9612, 9588, 9563, 9537, 9510, 9483, 9455, 9426, 9396,
		9366, 9335, 9304, 9271, 9238, 9205, 9170, 9135, 9099, 9063, 9025, 8987, 8949, 8910, 8870, 8829, 8788, 8746, 8703, 8660,
		8616, 8571, 8526, 8480, 8433, 8386, 8338, 8290, 8241, 8191, 8141, 8090, 8038, 7986, 7933, 7880, 7826, 7771, 7716, 7660,
		7604, 7547, 7489, 7431, 7372, 7313, 7253, 7193, 7132, 7071, 7009, 6946, 6883, 6819, 6755, 6691, 6626, 6560, 6494, 6427,
		6360, 6293, 6225, 6156, 6087, 6018, 5948, 5877, 5807, 5735, 5664, 5591, 5519, 5446, 5372, 5299, 5224, 5150, 5075, 5000,
		4924, 4848, 4771, 4694, 4617, 4539, 4461, 4383, 4305, 4226, 4146, 4067, 3987, 3907, 3826, 3746, 3665, 3583, 3502, 3420,
		3338, 3255, 3173, 3090, 3007, 2923, 2840, 2756, 2672, 2588, 2503, 2419, 2334, 2249, 2164, 2079, 1993, 1908, 1822, 1736,
		1650, 1564, 1478, 1391, 1305, 1218, 1132, 1045, 958, 871, 784, 697, 610, 523, 436, 348, 261, 174, 87, 0,
		-87, -174, -261, -348, -436, -523, -610, -697, -784, -871, -958, -1045, -1132, -1218, -1305, -1391, -1478, -1564, -1650, -1736,
		-1822, -1908, -1993, -2079, -2164, -2249, -2334, -2419, -2503, -2588, -2672, -2756, -2840, -2923, -3007, -3090, -3173, -3255, -3338, -3420,
		-3502, -3583, -3665, -3746, -3826, -3907, -3987, -4067, -4146, -4226, -4305, -4383, -4461, -4539, -4617, -4694, -4771, -4848, -4924, -5000,
		-5075, -5150, -5224, -5299, -5372, -5446, -5519, -5591, -5664, -5735, -5807, -5877, -5948, -6018, -6087, -6156, -6225, -6293, -6360, -6427,
		-6494, -6560, -6626, -6691, -6755, -6819, -6883, -6946, -7009, -7071, -7132, -7193, -7253, -7313, -7372, -7431, -7489, -7547, -7604, -7660,
		-7716, -7771, -7826, -7880, -7933, -7986, -8038, -8090, -8141, -8191, -8241, -8290, -8338, -8386, -8433, -8480, -8526, -8571, -8616, -8660,
		-8703, -8746, -8788, -8829, -8870, -8910, -8949, -8987, -9025, -9063, -9099, -9135, -9170, -9205, -9238, -9271, -9304, -9335, -9366, -9396,
		-9426, -9455, -9483, -9510, -9537, -9563, -9588, -9612, -9636, -9659, -9681, -9702, -9723, -9743, -9762, -9781, -9799, -9816, -9832, -9848,
		-9862, -9876, -9890, -9902, -9914, -9925, -9935, -9945, -9953, -9961, -9969, -9975, -9981, -9986, -9990, -9993, -9996, -9998, -9999, -10000
};

//_______________________________________________________________________________________________________
void init_display(int clk_hands) {
  c_hands = clk_hands;
  HAL_LCD_initDisplay();

  if (res == 96) {
    Graphics_initContext(&g_sContext, &g_sharp96x96LCD);
  } else if (res == 128) {
    Graphics_initContext(&g_sContext, &g_sharp128x128LCD);
  } else {
	debug_print(DBG_INFO, "[DSP] Non-valid resolution! (%i)\n", res);
    return;
  }

  Graphics_setForegroundColor(&g_sContext, ClrWhite);
  Graphics_setBackgroundColor(&g_sContext, ClrBlack);
  Graphics_setFont(&g_sContext, &g_sFontFixed6x8);
  Graphics_clearDisplay(&g_sContext);
  Graphics_flushBuffer(&g_sContext);

  debug_print(DBG_INFO, "[DSP] Initialized.\n");
}

//_______________________________________________________________________________________________________
void clear_display() {
	 Sharp128x128_ClearBuffer();
}

//_______________________________________________________________________________________________________
void flush() {
	Graphics_flushBuffer(&g_sContext);
}

//_______________________________________________________________________________________________________
void print_string(char* s, int x, int y, _Bool centered, _Bool opaque) {
  if (centered) {
    if (opaque)
      Graphics_drawStringCentered(&g_sContext, s, AUTO_STRING_LENGTH, x, y, OPAQUE_TEXT);
    else
      Graphics_drawStringCentered(&g_sContext, s, AUTO_STRING_LENGTH, x, y, TRANSPARENT_TEXT);
  } else {
    if (opaque)
      Graphics_drawString(&g_sContext, s, AUTO_STRING_LENGTH, x, y, OPAQUE_TEXT);
    else
      Graphics_drawString(&g_sContext, s, AUTO_STRING_LENGTH, x, y, TRANSPARENT_TEXT);
  }
}

//_______________________________________________________________________________________________________
void stop_display() {
  Display_Stop();
  debug_print(DBG_INFO, "[DSP] Stopped.\n");
}

//_______________________________________________________________________________________________________
void analogClock(struct tm * timeinfo) {
  int cX = (res/2); // center X
  int cY = (res/2); // center Y
  int r = (res-1)/2; // radius
  int margin = 2; // margin from outer circle

  // draw static part of clock face
  Graphics_drawCircle(&g_sContext, cX, cY, r); // outer circle
  Graphics_drawCircle(&g_sContext, cX, cY, r+1); // outer circle
  Graphics_drawCircle(&g_sContext, cX, cY, r+2); // outer circle

  // ticks
  int i;
  for (i = 1; i <= 60; ++i) {
    //float angle = -PI/2.0+((i/60.0)*2.0*PI);
    int x1 = cX + ((sin10000angle[(12*i+180)%720] * (r - margin))/10000);
    int y1 = cY + ((sin10000angle[12*i] * (r - margin))/10000);
    if (i % 15 == 0) { // major ticks, every 15 minutes
      int x2 = cX + ((sin10000angle[(12*i+180)%720] * (r - margin - 6))/10000);
      int y2 = cY + ((sin10000angle[12*i] * (r - margin - 6))/10000);
      Graphics_drawLine(&g_sContext, x1, y1, x2, y2);
    } else if (i % 5 == 0) { // minor ticks, every 5 minutes
      int x2 = cX + ((sin10000angle[(12*i+180)%720] * (r - margin - 4))/10000);
      int y2 = cY + ((sin10000angle[12*i] * (r - margin - 4))/10000);
      Graphics_drawLine(&g_sContext, x1, y1, x2, y2);
    } else { // single second ticks
      //int x2 = cX + ((sin10000angle[(12*i+180)%720] * (r - margin - 1))/10000);
      //int y2 = cY + ((sin10000angle[12*i] * (r - margin - 1))/10000);
      //Graphics_drawLine(&g_sContext, x1, y1, x2, y2);
    }
  }

  // draw digital time
  char timestr[6];
  mcu_snprintf(timestr, 5, "%d%d:%d%d", timeinfo->tm_hour/10, timeinfo->tm_hour%10, timeinfo->tm_min/10, timeinfo->tm_min%10);
  print_string(timestr, cY, 25, true, true);

  // draw clock hands
  int x1 = cX;
  int y1 = cY;
  int x2 = 0;
  int y2 = 0;
  int hour_len = (res==96)?20:30;
  int min_len  = (res==96)?30:45;

  // hour (+ interpolation based on current minute)
  x2 = cX + (sin10000angle[((timeinfo->tm_hour*60+timeinfo->tm_min)+180)%720] * hour_len)/10000;
  y2 = cY + (sin10000angle[(timeinfo->tm_hour*60+timeinfo->tm_min)%720] * hour_len)/10000;
  Graphics_drawLine(&g_sContext, x1-1, y1, x2, y2);
  Graphics_drawLine(&g_sContext, x1, y1-1, x2, y2);
  Graphics_drawLine(&g_sContext, x1+1, y1, x2, y2);
  Graphics_drawLine(&g_sContext, x1, y1+1, x2, y2);

  // draw minute hand
  if (c_hands > 1) {
    x2 = cX + (sin10000angle[((12*timeinfo->tm_min)+180)%720] * min_len)/10000;
    y2 = cY + (sin10000angle[12*timeinfo->tm_min] * min_len)/10000;
    Graphics_drawLine(&g_sContext, x1-1, y1, x2, y2);
    Graphics_drawLine(&g_sContext, x1, y1-1, x2, y2);
    Graphics_drawLine(&g_sContext, x1+1, y1, x2, y2);
    Graphics_drawLine(&g_sContext, x1, y1+1, x2, y2);
  }

  // draw second hand
  if (c_hands > 2) {
    csecond = timeinfo->tm_sec;
    x2 = cX + (sin10000angle[((12*timeinfo->tm_sec)+180)%720] * 35)/10000;
    y2 = cY + (sin10000angle[12*timeinfo->tm_sec] * 35)/10000;
    Graphics_drawLine(&g_sContext, x1, y1, x2, y2);
  }
}

//_______________________________________________________________________________________________________
void batteryCharge(int charge, _Bool charging) {
  int i;
  char batstr[5];
  if ((charge/10)%10 == 0 && charge/100 == 0) mcu_snprintf(batstr, 4, "  %d%%", charge);
  else if (charge/100 == 0) mcu_snprintf(batstr, 4, " %d%%", charge);
  else mcu_snprintf(batstr, 4, "%d%%", charge);

  if (res==96) {
    Graphics_drawLine(&g_sContext,90,9,92,9);
    Graphics_drawLine(&g_sContext,88,10,94,10);
    Graphics_drawLine(&g_sContext,88,11,88,20);
    Graphics_drawLine(&g_sContext,94,11,94,20);
    if (charging) {
        Graphics_drawLine(&g_sContext,89,20,93,20);
    	// draw charge symbol
        Graphics_drawLine(&g_sContext,90,15,92,15);
        Graphics_drawPixel(&g_sContext,92,12);
        Graphics_drawPixel(&g_sContext,91,13);
        Graphics_drawPixel(&g_sContext,90,14);
        Graphics_drawPixel(&g_sContext,92,16);
        Graphics_drawPixel(&g_sContext,91,17);
        Graphics_drawPixel(&g_sContext,90,18);

    } else {
      for (i=0; i<(charge/10); i++)
        Graphics_drawLine(&g_sContext,89,20-i,93,20-i);
    }
    print_string(batstr, 84, 3, true, true);
  }
  else if (res==128) {
    Graphics_drawLine(&g_sContext,120,9,124,9);
    Graphics_drawLine(&g_sContext,120,10,124,10);
    Graphics_drawLine(&g_sContext,118,11,126,11);
    Graphics_drawLine(&g_sContext,118,12,118,25);
    Graphics_drawLine(&g_sContext,126,12,126,25);
    if (charging) {
        Graphics_drawLine(&g_sContext,119,25,125,25);
    	// draw charge symbol
        Graphics_drawLine(&g_sContext,120,18,124,18);
        Graphics_drawLine(&g_sContext,120,19,124,19);
        Graphics_drawPixel(&g_sContext,124,14);
        Graphics_drawPixel(&g_sContext,123,15);
        Graphics_drawPixel(&g_sContext,123,16);
        Graphics_drawPixel(&g_sContext,122,16);
        Graphics_drawPixel(&g_sContext,122,17);
        Graphics_drawPixel(&g_sContext,121,17);
        Graphics_drawPixel(&g_sContext,123,20);
        Graphics_drawPixel(&g_sContext,122,20);
        Graphics_drawPixel(&g_sContext,122,21);
        Graphics_drawPixel(&g_sContext,121,21);
        Graphics_drawPixel(&g_sContext,121,22);
        Graphics_drawPixel(&g_sContext,120,23);
    } else {
      for (i=0; i<((3*charge)/20); i++)
        Graphics_drawLine(&g_sContext,119,25-i,125,25-i);
    }
    print_string(batstr, 117, 3, true, true);
  }
}

//_______________________________________________________________________________________________________
void warning(_Bool warn) {
	if (!warn)
		return;

	Graphics_drawCircle(&g_sContext, 9, 10, 5);
	Graphics_drawPixel(&g_sContext,9,7);
	Graphics_drawPixel(&g_sContext,9,8);
	Graphics_drawPixel(&g_sContext,9,9);
	Graphics_drawPixel(&g_sContext,9,10);
	Graphics_drawPixel(&g_sContext,9,12);
	Graphics_drawPixel(&g_sContext,9,13);
	Graphics_drawPixel(&g_sContext,10,7);
	Graphics_drawPixel(&g_sContext,10,8);
	Graphics_drawPixel(&g_sContext,10,9);
	Graphics_drawPixel(&g_sContext,10,10);
	Graphics_drawPixel(&g_sContext,10,12);
	Graphics_drawPixel(&g_sContext,10,13);
}

//_______________________________________________________________________________________________________
_Bool DSPChanges(struct tm * timeinfo, int charge, _Bool charging) {
	if ((c_hands<3) && (chour == timeinfo->tm_hour) && (cminute == timeinfo->tm_min) && (ccharge == charge) && (ccharging == charging)) {
		return(0); // no need to update
	} else if ((c_hands>2) && (cminute == timeinfo->tm_min) && (csecond == timeinfo->tm_sec) && (ccharge == charge) && (ccharging == charging)) {
		return(0); // no need to update
	}

	chour = timeinfo->tm_hour;
	cminute = timeinfo->tm_min;
	csecond = timeinfo->tm_sec;
	ccharge = charge;
	ccharging = charging;

	return(1);
}
