#include "mcu_api.h"
#include "mcu_errno.h"
// #include "grlib.h"
#include "LcdDriver.h"

#include <stdint.h>

// SPI ports
int SCS = 165; //SPI Chip Select
int VDD = 44; //VDD (DISP)
int SI = 115; //MOSI
int SCLK = 109; //SCLK //109

//_______________________________________________________________________________________________________
void GPIO_Init(void) {
	gpio_setup(SCS, 1); // SCS as output
	gpio_setup(VDD, 1); // VDD as output
}

//_______________________________________________________________________________________________________
void SPI_Init(void) {
	gpio_setup(SI, 1); // SI as output
	gpio_setup(SCLK, 1); // SCLK as output
}

//_______________________________________________________________________________________________________
void PWM_Init(void) {
	// PWM0, duty cycle in ns, time period in ns
	int pwm = pwm_configure(0, DUTY_CYCLE*TIME_PERIOD*1000000, TIME_PERIOD*1000000);
	if(pwm > 0) {
		debug_print(DBG_ERROR, "PWM initialization failed, exit...\n");
		// exit(1);
	}
}

//_______________________________________________________________________________________________________
void Display_Init(void) {
	gpio_write(SCS, 1); //Select SHARP Display
	SPI_writeByte(MLCD_CM); //Clear SHARP Display
	SPI_writeByte(MLCD_TR); //Send Trailer
	gpio_write(SCS, 0); //Deselect SHARP Display
}

//_______________________________________________________________________________________________________
void PWM_Run(void) {
	pwm_enable(0);
	debug_print(DBG_INFO, "PWM running\n");
}

//_______________________________________________________________________________________________________
void Display_Stop(void) {
  //reset all GPIOs to '0'
  gpio_write(SCS, 0);
  gpio_write(VDD, 0);

  //close all Peripherals
  pwm_disable(0);
}

//_______________________________________________________________________________________________________
void HAL_LCD_initDisplay(void) {
	GPIO_Init();
	mcu_sleep(10);
	HAL_LCD_enableDisplay();
	mcu_sleep(10);
	SPI_Init();
	mcu_sleep(10);
	PWM_Init();
	mcu_sleep(10);
	PWM_Run();

//	Display_Init();
}

//_______________________________________________________________________________________________________
void HAL_LCD_writeCommandOrData(uint16_t command) {
	SPI_writeByte(command);
}

//_______________________________________________________________________________________________________
void HAL_LCD_clearCS(void) {
	mcu_delay(2);
	gpio_write(SCS, 0);
}

//_______________________________________________________________________________________________________
void HAL_LCD_setCS(void) {
	gpio_write(SCS, 1);
	mcu_delay(6);
}

//_______________________________________________________________________________________________________
void HAL_LCD_waitUntilLcdWriteFinish(void) {
}

//_______________________________________________________________________________________________________
void HAL_LCD_disableDisplay(void) {
	gpio_write(VDD, 0);
}

//_______________________________________________________________________________________________________
void HAL_LCD_enableDisplay(void) {
	gpio_write(VDD, 1);
}

//_______________________________________________________________________________________________________
void HAL_LCD_prepareMemoryWrite() {
}

//_______________________________________________________________________________________________________
void HAL_LCD_finishMemoryWrite() {
}

//_______________________________________________________________________________________________________
void HAL_LCD_displayMode() {
	HAL_LCD_setCS();
	SPI_writeByte(MLCD_DM);
	SPI_writeByte(MLCD_TR);
	HAL_LCD_clearCS();
}

/*
 * mcu specific functions
 */

//_______________________________________________________________________________________________________
void SPI_writeByte(uint8_t data) {
	int i;
	gpio_write(SCLK, 0);
	for(i = 0; i < 8; i++) {
		gpio_write(SI, (data & (1<<7)) ? 1 : 0);
		data <<= 1;
		gpio_write(SCLK, 1);
		gpio_write(SCLK, 0);
	}
}
