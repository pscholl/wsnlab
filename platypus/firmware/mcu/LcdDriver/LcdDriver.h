#ifndef __LCDDRIVER_H__
#define __LCDDRIVER_H__

#include <stdint.h>

// Non-volatile Memory used to store DisplayBuffer
//#define NON_VOLATILE_MEMORY_BUFFER
#ifdef NON_VOLATILE_MEMORY_BUFFER
//#define USE_FLASH_BUFFER
#define NON_VOLATILE_MEMORY_ADDRESS			0xf400
#endif //NON_VOLATILE_MEMORY_BUFFER


// edison specific definitions

//SPI Clock frequency in Hz
#define fSCLK 2222000

//Time Period in ms
#define TIME_PERIOD 18

//Duty Cycle(0.0f - 1.0f)
#define DUTY_CYCLE 0.5f

//LCD Commands
#define MLCD_WR 0x80 //MLCD write line command
#define MLCD_CM 0x20 //MLCD clear memory command
#define MLCD_TR 0x00 //MLCD Trailer
#define MLCD_DM 0x00 //MLCD Display Mode (maintain display)


//*****************************************************************************
//
// Prototypes for the globals exported by this driver.
//
//*****************************************************************************
extern void HAL_LCD_initDisplay(void);
extern void HAL_LCD_writeCommandOrData(uint16_t command);
extern void HAL_LCD_clearCS(void);
extern void HAL_LCD_setCS(void);
extern void HAL_LCD_prepareMemoryWrite(void);
extern void HAL_LCD_finishMemoryWrite(void);
extern void HAL_LCD_waitUntilLcdWriteFinish(void);
extern void HAL_LCD_disableDisplay(void);
extern void HAL_LCD_enableDisplay(void);

extern void HAL_LCD_displayMode(void);

// edison specific functions
extern void GPIO_Init(void);
extern void SPI_Init(void);
extern void PWM_Init(void);
extern void Display_Init(void);
extern void PWM_Run(void);
extern void Display_Stop(void);

// mcu specific functions
void SPI_writeByte(uint8_t data);

#endif // __LCDDRIVER_H__
