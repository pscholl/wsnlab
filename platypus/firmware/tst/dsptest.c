#include "LcdDriver.h"
#include "Sharp96x96.h"
#include "grlib.h"

int main(int argc, const char* argv[]) {
  HAL_LCD_initDisplay(); // init display
  tContext g_sContext;
  Graphics_initContext(&g_sContext, &g_sharp96x96LCD); // init display context

  Graphics_setForegroundColor(&g_sContext, ClrWhite); // foreground color (text, drawings, etc.)
  Graphics_setBackgroundColor(&g_sContext, ClrBlack); // background color
  Graphics_setFont(&g_sContext, &g_sFontFixed6x8); // font (see grlib font folder)

  Graphics_clearDisplay(&g_sContext); // clear the display
  Graphics_flushBuffer(&g_sContext); // flush the changes (clear at this point)

  Graphics_drawLine(&g_sContext, 25, 11, 80, 90); // example for a line
  Graphics_drawCircle(&g_sContext, 50, 50, 20); // example for a circle
  Graphics_drawString(&g_sContext, "hello world", 12, 20, 40, OPAQUE_TEXT); // example for some text

  Graphics_flushBuffer(&g_sContext); // flush the changes (draws)
  printf("Done\n");

  return 0;
}
