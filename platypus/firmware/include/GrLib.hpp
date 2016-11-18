#ifndef GRLIB_HPP_
#define GRLIB_HPP_

#include <string>
#include "grlib.h"

class GrContext : Graphics_Context {
public:
	GrContext(const Graphics_Display &display) {
		Graphics_initContext(this, &display);
	}

	void drawCircle(int32_t x, int32_t y, int32_t lRadius) {
		Graphics_drawCircle(this, x, y, lRadius);
	}
	void fillCircle(int32_t x, int32_t y, int32_t lRadius) {
		Graphics_fillCircle(this, x, y, lRadius);
	}
	void setClipRegion(Graphics_Rectangle &rect) {
		Graphics_setClipRegion(this, &rect);
	}
	void drawImage(const Graphics_Image &pBitmap, int16_t x, int16_t y) {
		Graphics_drawImage(this, &pBitmap, x, y);
	}
	void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
		Graphics_drawLine(this, x1, y1, x2, y2);
	}
	void drawLineH(int32_t x1, int32_t x2, int32_t y) {
		Graphics_drawLineH(this, x1, x2, y);
	}
	void drawLineV(int32_t x, int32_t y1, int32_t y2) {
		Graphics_drawLineV(this, x, y1, y2);
	}
	void drawRectangle(const Graphics_Rectangle &rect) {
		Graphics_drawRectangle(this, &rect);
	}
	void fillRectangle(const Graphics_Rectangle &rect) {
		Graphics_fillRectangle(this, &rect);
	}
	void drawString(std::string string, int32_t x, int32_t y,
			bool opaque) {
		Graphics_drawString(this, string.data(), string.length(), x, y, opaque);
	}
	void drawStringCentered(std::string string, int32_t x, int32_t y,
			bool opaque) {
		Graphics_drawStringCentered(this, string.data(), string.length(), x, y, opaque);
	}
	int32_t getStringWidth(std::string string) {
		return Graphics_getStringWidth(this, string.data(), string.length());
	}
	void setBackgroundColor(int32_t value) {
		Graphics_setBackgroundColor(this, value);
	}
	uint16_t getDisplayWidth() {
		return Graphics_getDisplayWidth(this);
	}
	uint16_t getDisplayHeight() {
		return Graphics_getDisplayHeight(this);
	}
	void setFont(const Graphics_Font &font) {
		Graphics_setFont(this, &font);
	}
	void setForegroundColor(int32_t value) {
		Graphics_setForegroundColor(this, value);
	}
//	void setForegroundColorTranslated(int32_t value);
	uint8_t getStringHeight() {
		return Graphics_getStringHeight(this);
	}
	uint8_t getStringMaxWidth() {
		return Graphics_getStringMaxWidth(this);
	}
	uint8_t getStringBaseline() {
		return Graphics_getStringBaseline(this);
	}
	void drawPixel(uint16_t x, uint16_t y) {
		Graphics_drawPixel(this, x, y);
	}
	void clearDisplay() {
		Graphics_clearDisplay(this);
	}
	void flushBuffer() {
		Graphics_flushBuffer(this);
	}
};

#endif /* GRLIB_HPP_ */
