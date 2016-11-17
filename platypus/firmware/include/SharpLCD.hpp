#ifndef SHARPLCD_HPP_
#define SHARPLCD_HPP_

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <mraa/gpio.hpp>
#include <mraa/pwm.hpp>
#include <mraa/spi.hpp>

#include "grlib.h"

class SharpLCD {
private:
	const static int cmd_clear = 0x20;
	const static int cmd_writeLine = 0x80;
	const static int cmd_trail = 0x00;

	Graphics_Display c; // contains width and height
	size_t bwidth; // width of a line in bytes (excluding line index and trailer)

	mraa::Gpio scs; // SPI Chip Select
	mraa::Gpio vdd; // power
	mraa::Pwm pwm;
	mraa::Spi spi;

	/**
	 * just pixel data, line by line
	 */
	std::vector<uint8_t> frameBuf;

	/**
	 * structure:
	 * <command>
	 * <line index><pixel data><trailer> (for each line)
	 * <trailer>
	 * Where command, line index, and trailers are 1 byte each.
	 */
	std::vector<uint8_t> cmdBuf;

	std::mutex refreshMutex;
	std::thread dispThread;
	std::condition_variable refreshCond;
	// thread has been started and has not been joined
	bool refreshEnabled;
	// thread is running
	bool refreshRunning;
	// thread should terminate
	bool refreshTerminate;
	// count frames to compute fps
	uint32_t frameCounter;


	size_t pixelIndex(uint16_t x, uint16_t y);
	void applyMask(size_t index, uint8_t mask, uint8_t color);

	void refreshDisplay(uint8_t *data, int len);

	static void drawPixel(void *tp, int16_t x, int16_t y, uint16_t value);
	static void drawMultiplePixel(void *tp, int16_t x, int16_t y, int16_t x0, int16_t count, int16_t bPP, const uint8_t *data, const uint32_t *pucPalette);
	static void drawLineH(void *tp, int16_t x1, int16_t x2, int16_t y, uint16_t value);
	static void drawLineV(void *tp, int16_t x, int16_t y1, int16_t y2, uint16_t value);
	static void fillRect(void *tp, const Graphics_Rectangle *rect, uint16_t value);
	static uint32_t translateColor(void *tp, uint32_t value);
	static void flushBuffer(void *tp);
	static void clearDisplay(void *tp, uint16_t value);

public:
	SharpLCD(int width, int height);
	~SharpLCD();

	void enable();
	void disable();

	uint32_t getFrameCounter();

	operator const Graphics_Display & () const {
		return this->c;
	}
	operator const Graphics_Display * () const {
		return &this->c;
	}
};





#endif /* SHARPLCD_HPP_ */
