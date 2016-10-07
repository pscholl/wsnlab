/*
* Author: Mareike Höchner, Sebastian Boettcher
*
* Light-To-Digital Converter interface (TSL2584TSV)
*
*/

#ifndef ldc_edison_h
#define ldc_edison_h


#define LDC_ENABLE      0x00
#define LDC_ATIME       0x01
#define LDC_WTIME       0x03
#define LDC_AILTL       0x04
#define LDC_AILTH       0x05
#define LDC_AIHTL       0x06
#define LDC_AIHTH       0x07
#define LDC_PERS        0x0C
#define LDC_CONFIG      0x0D
#define LDC_CONTROL     0x0F
#define LDC_ID          0x12
#define LDC_STATUS      0x13
#define LDC_DATA0LOW    0x14
#define LDC_DATA0HIGH   0x15
#define LDC_DATA1LOW    0x16
#define LDC_DATA1HIGH   0x17

#define LDC_I2C_ADDRESS 0x39

#define INTEGRATION_TIMING 0x6C  // ca. 400ms
#define ANALOG_GAIN 0x02  // 16x



void setupLDC();
// set the I2C address of the device
void setAddress(uint8_t i2c_addr);
// reset and initialize the device
void ldc_init();

// get the current values from the ADCs, in an array [ADC0,ADC1]
void getADC(int16_t *data);

// single read from the specified register
uint8_t ldc_readRegister(uint8_t addr, uint8_t i2c);
//single write to the specified register
void ldc_writeRegister(uint8_t addr, uint8_t data, uint8_t i2c);


uint8_t m_ldc_address;
_Bool m_error;

#endif // ldc_edison_h

