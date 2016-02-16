/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Name of file: SHARPdisplay.c
Date: 01/03/2015
Purpose: Test SHARP LS013B4DN04 Memory Display
Hardware: Intel Edison(Mini Breakout Board)
Interface: TI SHARP Memory LCD Booster Pack
Peripherals: GPIOs, SPI and PWM
Compiler: GCC (mraa Library v0.6.1)
Project: Hedgehog v2.0
Author: Frederic. P
Guide: Prof. Kristof van Laerhoven 
Note:
fSCLK --> Typical: 0.5 MHz, Max: 1 MHz
fVCOM --> Min: 0.5 Hz, Max: 30 Hz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

//MRAA, GCC and Linux Kernel headers
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "mraa.h"

//Application headers
#include "SHARPdisplay.h"

//SPI Clock frequency in Hz
#define fSCLK 1000000

//Time Period in ms
#define TIME_PERIOD 150

//Duty Cycle(0.0f - 1.0f)
#define DUTY_CYCLE 0.5f

//LCD Commands
#define MLCD_WR 0x80 //MLCD write line command
#define MLCD_CM 0x20 //MLCD clear memory command
#define MLCD_TR 0x00 //MLCD Trailer

//LCD resolution
#define MLCD_XRES 96 //pixels per horizontal line
#define MLCD_YRES 96 //pixels per vertical line
#define MLCD_BYTES MLCD_XRES / 8 //number of bytes in a line(96/8 = MLCD_BYTES)
#define MLCD_BUF_SIZE MLCD_YRES * MLCD_BYTES //(96*MLCD_BYTES = 1152)

//Function Declarations
void GPIO_Init(void);
void SPI_Init(void);
void PWM_Init(void);
void Display_Init(void);
void Display_Image(char image[]);
void PWM_Run(void);
void sig_handler(int signum);
static unsigned char reverse_byte(unsigned char x);

sig_atomic_t volatile isrunning = 1;
mraa_gpio_context SCS; //SPI Chip Select
mraa_spi_context spi;
mraa_pwm_context pwm;

int main(int argc, char **argv)
{
	printf("-------------------------------------------------------\n");
	printf("Welcome to SHARP Memory Display demo using Intel Edison\n");
	printf("-------------------------------------------------------\n");
	sleep(1);
	
	signal(SIGINT, &sig_handler);
	
	GPIO_Init();
	sleep(1);
	SPI_Init();
	sleep(1);
	PWM_Init();
	sleep(1);
	
	PWM_Run(); 
	
	Display_Init();
	printf("SHARP Display initialized\n");
	
	while(isrunning)
	{
		//Loop Forever
                Display_Init();
		printf("UNI FREIBURG\n");
		Display_Image("Uni_Freiburg");
		sleep(5);

                Display_Init();
                printf("INTEL ATOM\n");
		Display_Image("Intel_Atom");
		sleep(5);

                Display_Init();
                printf("NIKOLA TESLA\n");
		Display_Image("Nikola_Tesla");
		sleep(5);

                Display_Init();
                printf("ALBERT EINSTEIN\n");
		Display_Image("Albert_Einstein");
		sleep(5);

                Display_Init();
                printf("WHITE TIGER\n");
		Display_Image("White_Tiger");
		sleep(5);
		
                Display_Init();
                printf("WALL-E\n");
		Display_Image("Wall_E");
		sleep(5);
	}
	
	//reset all GPIOs to '0'
	mraa_gpio_write(SCS, 0);
	
	//close all GPIOs
	mraa_gpio_close(SCS);
	
	//close all Peripherals
	mraa_spi_stop(spi);
	mraa_pwm_close(pwm);
	
	fprintf(stdout, "\nSHARP Display exited\n");
	
	return MRAA_SUCCESS;
}

void GPIO_Init(void)
{
	mraa_init();
	
	//J18-2 --> GPIO-165
	SCS = mraa_gpio_init(15); 
	printf("Ports initialized\n");
	
	mraa_gpio_mode(SCS, MRAA_GPIO_PULLUP);
	printf("Ports modes set\n");
	
	mraa_gpio_dir(SCS, MRAA_GPIO_OUT);
	printf("Port directions set\n");
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
	printf("SPI initialized successfully\n");
	
	mraa_spi_frequency (spi, fSCLK);
	printf("SPI clock frequency set to %iHz\n", fSCLK);
	
	/*
	MRAA_SPI_MODE0 	
	CPOL = 0, CPHA = 0, 
	Clock idle low, data is clocked in on rising edge, output data (change) on falling edge 
	*/
	mraa_spi_mode(spi, MRAA_SPI_MODE0);
	printf("SPI set to MODE0\n");
	
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

	printf("PWM initialized successfully\n");
}

void Display_Init(void)
{
	mraa_gpio_write(SCS, 1); //Select SHARP Display
	mraa_spi_write(spi, MLCD_CM); //Clear SHARP Display
	mraa_spi_write(spi, MLCD_TR); //Send Trailer
	mraa_gpio_write(SCS, 0); //Deselect SHARP Display
}

void Display_Image(char image[])
{
	static unsigned char line_no;
	static unsigned int pixel;
	
	mraa_gpio_write(SCS, 1); //Select SHARP Display
	mraa_spi_write(spi, MLCD_WR); //Send Write Command	
	
	if(strcmp(image, "Uni_Freiburg") == 0)
	{
		for(line_no = 1; line_no <= MLCD_YRES; line_no++)
		{
			mraa_spi_write(spi, reverse_byte(line_no)); //Send Line Number
			
        		for(pixel = ((line_no - 1) * MLCD_BYTES); pixel < (line_no * MLCD_BYTES); pixel++)
        		{
				mraa_spi_write(spi, BMP_Uni_Freiburg[pixel]); //Send Data
        		}
	
		mraa_spi_write(spi, MLCD_TR); //Send Trailer
		}
	}

	else if(strcmp(image, "Intel_Atom") == 0)
	{
		for(line_no = 1; line_no <= MLCD_YRES; line_no++)
		{
			mraa_spi_write(spi, reverse_byte(line_no)); //Send Line Number
			
        		for(pixel = ((line_no - 1) * MLCD_BYTES); pixel < (line_no * MLCD_BYTES); pixel++)
        		{
				mraa_spi_write(spi, BMP_Intel_Atom[pixel]); //Send Data
        		}
	
		mraa_spi_write(spi, MLCD_TR); //Send Trailer
		}
	}

	else if(strcmp(image, "Nikola_Tesla") == 0)
	{
		for(line_no = 1; line_no <= MLCD_YRES; line_no++)
		{
			mraa_spi_write(spi, reverse_byte(line_no)); //Send Line Number
			
        		for(pixel = ((line_no - 1) * MLCD_BYTES); pixel < (line_no * MLCD_BYTES); pixel++)
        		{
				mraa_spi_write(spi, BMP_Nikola_Tesla[pixel]); //Send Data
        		}
	
		mraa_spi_write(spi, MLCD_TR); //Send Trailer
		}
	}

	else if(strcmp(image, "Albert_Einstein") == 0)
	{
		for(line_no = 1; line_no <= MLCD_YRES; line_no++)
		{
			mraa_spi_write(spi, reverse_byte(line_no)); //Send Line Number
			
        		for(pixel = ((line_no - 1) * MLCD_BYTES); pixel < (line_no * MLCD_BYTES); pixel++)
        		{
				mraa_spi_write(spi, BMP_Albert_Einstein[pixel]); //Send Data
        		}
	
		mraa_spi_write(spi, MLCD_TR); //Send Trailer
		}
	}

	else if(strcmp(image, "White_Tiger") == 0)
	{
		for(line_no = 1; line_no <= MLCD_YRES; line_no++)
		{
			mraa_spi_write(spi, reverse_byte(line_no)); //Send Line Number
			
        		for(pixel = ((line_no - 1) * MLCD_BYTES); pixel < (line_no * MLCD_BYTES); pixel++)
        		{
				mraa_spi_write(spi, BMP_White_Tiger[pixel]); //Send Data
        		}
	
		mraa_spi_write(spi, MLCD_TR); //Send Trailer
		}
	}

	else if(strcmp(image, "Wall_E") == 0)
	{
		for(line_no = 1; line_no <= MLCD_YRES; line_no++)
		{
			mraa_spi_write(spi, reverse_byte(line_no)); //Send Line Number
			
        		for(pixel = ((line_no - 1) * MLCD_BYTES); pixel < (line_no * MLCD_BYTES); pixel++)
        		{
				mraa_spi_write(spi, BMP_Wall_E[pixel]); //Send Data
        		}
	
		mraa_spi_write(spi, MLCD_TR); //Send Trailer
		}
	}
	
	mraa_spi_write(spi, MLCD_TR); //Send last Trailer
	mraa_gpio_write(SCS, 0); //Deselect SHARP Display
}

void PWM_Run(void)
{
	mraa_pwm_period_ms(pwm, TIME_PERIOD);
	printf("Time Period set to %dms\n", TIME_PERIOD);
  
	mraa_pwm_enable(pwm,1);
	printf("PWM enabled\n");
  
	mraa_pwm_write(pwm, DUTY_CYCLE);
	printf("Duty Cycle set to %f%%\n", (100 * DUTY_CYCLE));

	printf("PWM running\n");	
}

//Signal Handler
void sig_handler(int signum)
{
	if(signum == SIGINT)
	{
		isrunning = 0;
	}
}