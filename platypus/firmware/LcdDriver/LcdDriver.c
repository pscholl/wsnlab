#include "grlib.h"
#include "LcdDriver.h"


mraa_gpio_context SCS; //SPI Chip Select
mraa_gpio_context VDD; //VDD
mraa_spi_context spi;
mraa_pwm_context pwm;


void GPIO_Init(void)
{
	mraa_init();

	//J18-2 --> GPIO-165
	SCS = mraa_gpio_init(15);
	//printf("Ports initialized\n");

  mraa_gpio_use_mmaped(SCS, 1);

	mraa_gpio_mode(SCS, MRAA_GPIO_PULLUP);
	//printf("Ports modes set\n");

	mraa_gpio_dir(SCS, MRAA_GPIO_OUT);
	//printf("Port directions set\n");

	VDD = mraa_gpio_init(31); // GP44
	mraa_gpio_mode(VDD, MRAA_GPIO_PULLUP);
	mraa_gpio_dir(VDD, MRAA_GPIO_OUT);
}

void SPI_Init(void)
{
	//MOSI --> J17-12 --> GPIO-115
	//SCK --> J17-11 -->GPIO-109
	spi = mraa_spi_init(1);

	if (spi == NULL)
	{
		printf("SPI initialization failed, check syslog for details, exit...\n");
		exit(1);
	}
	//printf("SPI initialized successfully\n");

	mraa_spi_frequency (spi, fSCLK);
	//printf("SPI clock frequency set to %iHz\n", fSCLK);

	/*
	MRAA_SPI_MODE0
	CPOL = 0, CPHA = 0,
	Clock idle low, data is clocked in on rising edge, output data (change) on falling edge
	*/
	mraa_spi_mode(spi, MRAA_SPI_MODE0);
	//printf("SPI set to MODE0\n");

	//LSB Transmission
	mraa_spi_lsbmode(spi, 0);
}

void PWM_Init(void)
{
	pwm = mraa_pwm_init(20); //J18-7 --> GPIO-12 --> PWM0

	if(pwm == NULL)
	{
		printf("PWM initialization failed, check syslog for details, exit...\n");
		exit(1);
	}

	//printf("PWM initialized successfully\n");
}

void Display_Init(void)
{
	mraa_gpio_write(SCS, 1); //Select SHARP Display
	mraa_spi_write(spi, MLCD_CM); //Clear SHARP Display
	mraa_spi_write(spi, MLCD_TR); //Send Trailer
	mraa_gpio_write(SCS, 0); //Deselect SHARP Display
}

void PWM_Run(void)
{
	mraa_pwm_period_ms(pwm, TIME_PERIOD);
	//printf("Time Period set to %dms\n", TIME_PERIOD);

	mraa_pwm_enable(pwm,1);
	//printf("PWM enabled\n");


	mraa_pwm_write(pwm, DUTY_CYCLE);
	//printf("Duty Cycle set to %f%%\n", (100 * DUTY_CYCLE));

	//printf("PWM running\n");
}

void Display_Stop(void)
{
  //reset all GPIOs to '0'
  mraa_gpio_write(SCS, 0);
  mraa_gpio_write(VDD, 0);
  //close all GPIOs
  mraa_gpio_close(SCS);
  mraa_gpio_close(VDD);
  //close all Peripherals
  mraa_spi_stop(spi);
  mraa_pwm_close(pwm);
}


void HAL_LCD_initDisplay(void)
{
	GPIO_Init();
	usleep(100000);
	HAL_LCD_enableDisplay();
	usleep(100000);
	SPI_Init();
	usleep(100000);
	PWM_Init();
	usleep(100000);

	PWM_Run();

	Display_Init();
}


void HAL_LCD_writeCommandOrData(uint16_t command)
{
  //mraa_spi_write(spi, MLCD_WR); //Send Write Command
	mraa_spi_write(spi, command);
	//mraa_spi_write(spi, MLCD_TR); //Send Trailer
}

void HAL_LCD_clearCS(void){
  usleep(2);
  mraa_gpio_write(SCS, 0);
}

void HAL_LCD_setCS(void){
  mraa_gpio_write(SCS, 1);
  usleep(6);
}

void HAL_LCD_waitUntilLcdWriteFinish(void)
{
}

void HAL_LCD_disableDisplay(void)
{
	mraa_gpio_write(VDD, 0);
}

void HAL_LCD_enableDisplay(void)
{
	mraa_gpio_write(VDD, 1);
}

void HAL_LCD_prepareMemoryWrite()
{
}

void HAL_LCD_finishMemoryWrite()
{
}


void HAL_LCD_displayMode()
{
	HAL_LCD_setCS();
	HAL_LCD_writeCommandOrData(MLCD_DM);
	HAL_LCD_writeCommandOrData(MLCD_TR);
	HAL_LCD_clearCS();
}