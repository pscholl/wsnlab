#include <chrono>
#include <iostream>

#include <mraa/common.hpp>

#include <boost/asio.hpp>

#include "SharpLCD.hpp"


static uint8_t reverseBits(uint8_t x) {
	x = ((x & 0xAA) >> 1) | ((x & 0x55) << 1);
	x = ((x & 0xCC) >> 2) | ((x & 0x33) << 2);
	x = ((x & 0xF0) >> 4) | ((x & 0x0F) << 4);
	return x;
}

static void mR(mraa::Result r) {
	if (r != mraa::SUCCESS) {
		mraa::printError(r);
		throw std::invalid_argument("MRAA error");
	}
}

static uint8_t pixelMask(int x) {
	return 0x80 >> (x % 8);
}

static uint8_t startMask(int x) {
	return 0xFF >> (x % 8);
}

static uint8_t endMask(int x) {
	return 0xFF80 >> (x % 8);
}

size_t SharpLCD::pixelIndex(uint16_t x, uint16_t y) {
	return bwidth * y + (x / 8);
}

void SharpLCD::applyMask(size_t index, uint8_t mask, uint8_t color) {
	// color is assumed to be 0x00(Black) or 0xFF (white)
	// this is ensured by translateColor
	uint8_t &p = frameBuf[index];
	p = (p | (mask & color)) & (~mask | color);
}

SharpLCD::SharpLCD(int width, int height) :	scs(15), vdd(31), pwm(20), spi(1) {
	c.size = sizeof(c);
	c.displayData = this;
	c.width = width;
	c.heigth = height;
	c.callPixelDraw = &SharpLCD::drawPixel;
	c.callPixelDrawMultiple = &SharpLCD::drawMultiplePixel;
	c.callLineDrawH = &SharpLCD::drawLineH;
	c.callLineDrawV = &SharpLCD::drawLineV;
	c.callRectFill = &SharpLCD::fillRect;
	c.callColorTranslate = &SharpLCD::translateColor;
	c.callFlush = &SharpLCD::flushBuffer;
	c.callClearDisplay = &SharpLCD::clearDisplay;

	bwidth = (width + 7) / 8;
	refreshEnabled = false;
	refreshRunning = false;
	refreshTerminate = false;
	frameCounter = 0;

	//GPIO Init
	mR(scs.useMmap(true));
	mR(scs.mode(mraa::MODE_PULLUP));
	mR(scs.dir(mraa::DIR_OUT));
	mR(vdd.mode(mraa::MODE_PULLUP));
	mR(vdd.dir(mraa::DIR_OUT));

	// Set SPI clock to 1MHz
	// LS013B4DN04: ƒSCLK: Typical 0.5MHz, Max 1.0MHz
	// LS013B7DH03: ƒSCLK: Typical 1.0MHz, Max 1.1MHz
	mR(spi.frequency(1000000));
	mR(spi.mode(mraa::SPI_MODE0));
	mR(spi.bitPerWord(8));
	mR(spi.lsbmode(false));

	// Set PWM to 60Hz
	// LS013B4DN04: fEXTCOMIN: Typical 1Hz, Max 60Hz
	// LS013B7DH03: fEXTCOMIN: Min 54Hz, Max 65Hz
	mR(pwm.period_us(16666));
	mR(pwm.pulsewidth_us(8333));

	cmdBuf[0].assign(2 + (2 + bwidth) * height, cmd_trail);
	// command
	cmdBuf[0][0] = cmd_writeLine;
	for (int row = 0; row < height; row++) {
		// line numbers
		cmdBuf[0][1 + row * (2 + bwidth)] = reverseBits(row + 1);
	}

	cmdBuf[1] = cmdBuf[0];
	cmdBufIndex = 0;

	frameBuf.assign(bwidth * height, 0xFF);
	flushBuffer(this);
}
SharpLCD::~SharpLCD() {
	refreshTerminate = true;
	if (refreshEnabled) {
		dispThread.join();
	}
	scs.write(false);
	vdd.write(false);
	pwm.enable(false);
}

void SharpLCD::enable() {
	if (refreshEnabled)
		return;
	// power up display
	mR(pwm.enable(true));
	mR(vdd.write(true));
	// start display thread
	refreshEnabled = true;
	refreshTerminate = false;

	dispThread = std::thread(&SharpLCD::refreshDisplay, this,
			cmdBuf[0].data(), cmdBuf[1].data(), cmdBuf[1].size());
}

void SharpLCD::disable() {
	refreshTerminate = true;
	if (refreshEnabled) {
		dispThread.join();
		refreshEnabled = false;
	}
	mR(vdd.write(false));
	mR(pwm.enable(false));
}

void SharpLCD::refreshDisplay(uint8_t *data0, uint8_t *data1, int len) {
	// for good measure, wait a few ms until things have settled
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	std::unique_lock<std::mutex> refreshLock(refreshMutex);
	refreshRunning = true;
	refreshCond.notify_all();

	uint8_t * const dataPtr[2] = {data0, data1};
	while (!refreshTerminate) {
		uint8_t index = cmdBufIndex;
		uint8_t *data = dataPtr[index];
		cmdBufUsed[index] = true;
		frameCounter++;
		refreshLock.unlock();
		std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
		scs.write(true);
		std::this_thread::sleep_for(std::chrono::microseconds(6));
		//TODO check for error
		spi.transfer(data, NULL, len);
		std::this_thread::sleep_for(std::chrono::microseconds(2));
		scs.write(false);
		refreshCond.notify_all();
		std::this_thread::sleep_until(t + std::chrono::microseconds(16666));
		refreshLock.lock();
	}
	refreshRunning = false;
	refreshCond.notify_all();
}


void SharpLCD::drawPixel(void *tp, int16_t x, int16_t y, uint16_t value) {
	SharpLCD &t = *(SharpLCD*) tp;
	t.applyMask(t.pixelIndex(x, y), pixelMask(x), value);
}

void SharpLCD::drawMultiplePixel(void *tp, int16_t x, int16_t y, int16_t x0, int16_t count, int16_t bPP, const uint8_t *data, const uint32_t *pucPalette) {
	// this doesn't seem to be needed for anything
	std::cerr << "drawMultiplePixel is not supported" << std::endl;
}

void SharpLCD::drawLineH(void *tp, int16_t x1, int16_t x2, int16_t y, uint16_t value) {
	SharpLCD &t = *(SharpLCD*) tp;
	size_t sidx = t.pixelIndex(x1, y);
	size_t eidx = t.pixelIndex(x2, y);
	uint8_t smask = startMask(x1);
	uint8_t emask = endMask(x2);
	if (sidx == eidx) {
		t.applyMask(sidx, smask & emask, value);
	} else {
		t.applyMask(sidx, smask, value);
		for (size_t i=sidx+1; i<eidx; i++) {
			t.applyMask(i, 0xFF, value);
		}
		t.applyMask(eidx, emask, value);
	}
}

void SharpLCD::drawLineV(void *tp, int16_t x, int16_t y1, int16_t y2, uint16_t value) {
	SharpLCD &t = *(SharpLCD*) tp;
	uint8_t mask = pixelMask(x);
	for (int16_t y = y1; y<=y2; y++) {
		t.applyMask(t.pixelIndex(x, y), mask, value);
	}
}

void SharpLCD::fillRect(void *tp, const Graphics_Rectangle *rect, uint16_t value) {
	for (int16_t y=rect->yMin; y<=rect->yMax; y++) {
		SharpLCD::drawLineH(tp, rect->xMin, rect->xMax, y, value);
	}
}

uint32_t SharpLCD::translateColor(void *tp, uint32_t value) {
	return value ? 0xFF : 0x00;
}

uint32_t SharpLCD::getFrameCounter() {
	std::unique_lock<std::mutex> refreshLock(refreshMutex);
	return this->frameCounter;
}

void sendFrameBuf(uint8_t *data, size_t size) {
	using boost::asio::ip::udp;
	static boost::asio::io_service io;
	static udp::socket s(io, udp::endpoint(udp::v4(), 0));
	static udp::endpoint t(boost::asio::ip::address::from_string("255.255.255.255"), 9090);
	s.send_to(boost::asio::buffer(data, size), t);
}

void SharpLCD::flushBuffer(void *tp) {
	SharpLCD &t = *(SharpLCD*) tp;
	uint8_t freeIndex = 1 - t.cmdBufIndex;
	// skip command and line number
	uint8_t *dst = t.cmdBuf[freeIndex].data()+2;
	uint8_t *src = t.frameBuf.data();
	for (uint16_t i=0; i<t.c.heigth; i++) {
		for (size_t j=0; j<t.bwidth; j++) {
			*dst++ = *src++;
		}
		dst += 2; // skip trailer and line number
	}
	sendFrameBuf(t.frameBuf.data(), t.bwidth*t.c.heigth);
	std::unique_lock<std::mutex> refreshLock(t.refreshMutex);
	t.cmdBufIndex = freeIndex;
	t.cmdBufUsed[freeIndex] = false;
	while (!t.cmdBufUsed[freeIndex] && t.refreshRunning) {
		t.refreshCond.wait(refreshLock);
	}
}

void SharpLCD::clearDisplay(void *tp, uint16_t value) {
	// value is assumed to be 0x00(Black) or 0xFF (white)
	// this is ensured by translateColor
	SharpLCD &t = *(SharpLCD*) tp;
	t.frameBuf.assign(t.frameBuf.size(), value);
}
