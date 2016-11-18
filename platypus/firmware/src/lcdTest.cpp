#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "SharpLCD.hpp"
#include "GrLib.hpp"

using std::chrono::steady_clock;

void testBasic1() {
	SharpLCD lcd(96, 96);
	lcd.enable();

	Graphics_Context g;
	Graphics_initContext(&g, lcd);
	Graphics_setBackgroundColor(&g, ClrBlack);
	Graphics_setForegroundColor(&g, ClrWhite);
	Graphics_setFont(&g, &g_sFontFixed6x8);
	Graphics_clearDisplay(&g);

	Graphics_Rectangle r;
	r.xMin = 10;
	r.xMax = 30;
	r.yMin = 40;
	r.yMax = 50;

	Graphics_drawCircle(&g, 20, 20, 10);
	Graphics_fillCircle(&g, 50, 20, 10);
	Graphics_drawRectangle(&g, &r);
	r.xMin += 30;
	r.xMax += 30;
	Graphics_fillRectangle(&g, &r);
	
	Graphics_drawLine(&g, 10, 20, 86, 76);

	Graphics_drawString(&g, "Hello World!", -1, 10, 60, false);
	Graphics_drawString(&g, "Hello World!", -1, 15, 65, true);
	Graphics_drawString(&g, "Hello World!", -1, 20, 70, false);

	Graphics_flushBuffer(&g);
	sleep(3);
}

void testBasic2() {
	SharpLCD lcd(96, 96);
	lcd.enable();

	GrContext g(lcd);
	g.setFont(g_sFontFixed6x8);
	g.setBackgroundColor(ClrWhite);
	g.setForegroundColor(ClrBlack);
	g.clearDisplay();

	Graphics_Rectangle r;
	r.xMin = 10;
	r.xMax = 30;
	r.yMin = 40;
	r.yMax = 50;

	g.drawCircle(20, 20, 10);
	g.fillCircle(50, 20, 10);
	g.drawRectangle(r);
	r.xMin += 30;
	r.xMax += 30;
	g.fillRectangle(r);

	g.drawLine(10, 20, 86, 76);

	g.drawString("Hello World!", 10, 60, false);
	g.drawString("Hello World!", 15, 65, true);
	g.drawString("Hello World!", 20, 70, false);

	g.flushBuffer();
	sleep(3);
}

void testFPS() {
	SharpLCD lcd(96, 96);
	lcd.enable();

	GrContext g(lcd);
	g.setBackgroundColor(ClrWhite);
	g.setForegroundColor(ClrBlack);

	steady_clock::time_point lastTime = steady_clock::now();
	uint32_t lastFC = lcd.getFrameCounter();

	const int radius=10;
	const int minX = radius;
	const int maxX = g.getDisplayWidth() - radius;
	const int maxY = g.getDisplayHeight() - radius;

	float posx = 30;
	float posy = 30;
	float speedx = 0.5;
	float speedy = 0;
	float accely = 0.11;

	while (true) {
		posx += speedx;
		posy += speedy;
		speedy += accely;

		if (posx < minX) {
			posx = 2*minX - posx;
			speedx = -speedx;
		}
		if (posx > maxX) {
			posx = 2*maxX - posx;
			speedx = -speedx;
		}
		if (posy > maxY) {
			//posy = 2*maxY - posy;
			//speedx = 1.0 * speedx;
			posy = maxY;
			speedy = -0.97 * speedy;
		}

		g.clearDisplay();
		g.fillCircle(posx, posy, 10);

		g.flushBuffer();

		steady_clock::time_point curTime = steady_clock::now();
		if (curTime - lastTime >= std::chrono::seconds(1)) {
			uint32_t curFC = lcd.getFrameCounter();
			printf("fps: %d\n", curFC - lastFC);
			lastTime = curTime;
			lastFC = curFC;
		}
	}
}

int main(void) {
	printf("Starting\n");
	testBasic1();
	testBasic2();
	testFPS();
	return 0;
}
