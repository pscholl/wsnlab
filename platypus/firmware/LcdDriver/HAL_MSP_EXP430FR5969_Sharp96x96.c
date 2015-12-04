//*****************************************************************************
//
// HAL_MSP-EXP430FR5969_Sharp96x96.c
//
//*****************************************************************************
//
//! \addtogroup display_api
//! @{
//
//*****************************************************************************

#include "grlib.h"
#include "HAL_MSP_EXP430FR5969_Sharp96x96.h"
//#include "driverlib.h"


mraa_gpio_context SCS; //SPI Chip Select
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
  //close all GPIOs
  mraa_gpio_close(SCS);
  //close all Peripherals
  mraa_spi_stop(spi);
  mraa_pwm_close(pwm);
}



//*****************************************************************************
//
//! Initializes the display driver.
//!
//! This function initializes the Sharp96x96 display. This function
//! configures the GPIO pins used to control the LCD display when the basic
//! GPIO interface is in use. On exit, the LCD has been reset and is ready to
//! receive command and data writes.
//!
//! \return None.
//
//*****************************************************************************
void HAL_LCD_initDisplay(void)
{
	GPIO_Init();
	usleep(100000);
	SPI_Init();
	usleep(100000);
	PWM_Init();
	usleep(100000);
	
	PWM_Run();

	Display_Init();

    /*
    // Configure LCD_SCLK_PIN for SPI_CLK mode
    GPIO_setAsPeripheralModuleFunctionOutputPin(LCD_SPI_CLK_PORT,
                LCD_SPI_CLK_PIN , LCD_SPI_CLK_PIN_FUNCTION);

    // Configure LCD_MOSI_PIN for SPI_MOSI mode
    GPIO_setAsPeripheralModuleFunctionOutputPin(LCD_SPI_SI_PORT,
    		LCD_SPI_SI_PIN , LCD_SPI_SI_PIN_FUNCTION);

    // Provide power to LCD
	GPIO_setAsOutputPin(LCD_POWER_PORT,
	        LCD_POWER_PIN);
	
	GPIO_setOutputHighOnPin(LCD_POWER_PORT,
            LCD_POWER_PIN);
				
	// Turn on DISP
	GPIO_setAsOutputPin(LCD_DISP_PORT,
	        LCD_DISP_PIN);
	
	GPIO_setOutputHighOnPin(LCD_DISP_PORT,
            LCD_DISP_PIN);		
	
			
	// Configure LCD_SPI_CS_PIN as output pin
    GPIO_setAsOutputPin(LCD_SPI_CS_PORT,
    		LCD_SPI_CS_PIN);
			
	HAL_LCD_clearCS();

	EUSCI_B_SPI_initMasterParam spiMasterParams=
	{
		EUSCI_B_SPI_CLOCKSOURCE_SMCLK,
		8000000,
		1000000,
		EUSCI_B_SPI_MSB_FIRST,
		EUSCI_B_SPI_PHASE_DATA_CAPTURED_ONFIRST_CHANGED_ON_NEXT,
		EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_LOW,
		EUSCI_B_SPI_3PIN
	};

    EUSCI_B_SPI_initMaster(LCD_EUSCI_BASE,
                    &spiMasterParams);

    EUSCI_B_SPI_enable(LCD_EUSCI_BASE);
*/
}



//*****************************************************************************
//
// Writes command or data to the LCD Driver
//
// \param ucCmdData is the 8 or 16 bit command to send to the LCD driver
// Uses the SET_LCD_DATA macro
//
// \return None
//
//*****************************************************************************
void HAL_LCD_writeCommandOrData(uint16_t command)
{
  //mraa_spi_write(spi, MLCD_WR); //Send Write Command
	mraa_spi_write(spi, command);
	//mraa_spi_write(spi, MLCD_TR); //Send Trailer
/*
	while (!EUSCI_B_SPI_getInterruptStatus(LCD_EUSCI_BASE,
	                    EUSCI_B_SPI_TRANSMIT_INTERRUPT));
	EUSCI_B_SPI_transmitData(LCD_EUSCI_BASE,command);
*/

}

//*****************************************************************************
//
// Clears CS line
//
// This macro allows to clear the Chip Select (CS) line
//
// \return None
//
//*****************************************************************************
void HAL_LCD_clearCS(void){
	mraa_gpio_write(SCS, 0);
	//GPIO_setOutputLowOnPin(LCD_SPI_CS_PORT, LCD_SPI_CS_PIN);
}

//*****************************************************************************
//
// Set CS line
//
// This macro allows to set the Chip Select (CS) line
//
// \return None
//
//*****************************************************************************
void HAL_LCD_setCS(void){
	mraa_gpio_write(SCS, 1);
	//GPIO_setOutputHighOnPin(LCD_SPI_CS_PORT, LCD_SPI_CS_PIN);
}

//*****************************************************************************
//
// Waits until the SPI communication with the LCD is finished a command to
// the LCD Driver
//
// \param None
//
// \return None
//*****************************************************************************
void HAL_LCD_waitUntilLcdWriteFinish(void)
{
	//while  (EUSCI_B_SPI_isBusy(LCD_EUSCI_BASE));
}

//*****************************************************************************
//
// Disables Shapr96x96 LCD
//
// \param None
//
// \return None
//*****************************************************************************
void HAL_LCD_disableDisplay(void)
{
	//GPIO_setOutputLowOnPin(LCD_DISP_PORT, LCD_DISP_PIN);
	//GPIO_setOutputLowOnPin(LCD_POWER_PORT, LCD_POWER_PIN);
}

//*****************************************************************************
//
// Enables Shapr96x96 LCD
//
// \param None
//
// \return None
//*****************************************************************************
void HAL_LCD_enableDisplay(void)
{
	//GPIO_setOutputHighOnPin(LCD_POWER_PORT, LCD_POWER_PIN);
	//GPIO_setOutputHighOnPin(LCD_DISP_PORT, LCD_DISP_PIN);

}


//*****************************************************************************
//
// Prepare to write memory
//
// This macro unlocks flash memory controller and
// sets access right on flash controller
//
// \return None
//
//*****************************************************************************
void HAL_LCD_prepareMemoryWrite()
{
}


//*****************************************************************************
//
// Finish memory writing
//
// This macro removes access rights on flash controller and
// locks flash memory controller.
//
// \return None
//
//*****************************************************************************
void HAL_LCD_finishMemoryWrite()
{
	//__no_operation();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
