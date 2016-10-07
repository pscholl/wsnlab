/*
* Author: Mareike Höchner, Sebastian Boettcher
*
* Light-To-Digital Converter interface (TSL2584TSV)
*
*/

#include "mcu_api.h"
#include "mcu_errno.h"
#include <stdint.h>
#include "./ldc_mcu.h"

/*
 * Initialization
 */

//_______________________________________________________________________________________________________
void setupLDC() {
  setAddress(LDC_I2C_ADDRESS);
  ldc_init();
  debug_print(DBG_INFO, "[LDC] Setup done.\n");
}

//_______________________________________________________________________________________________________
void setAddress(uint8_t i2c_addr) {
  m_ldc_address = i2c_addr;
}

//_______________________________________________________________________________________________________
void ldc_init() {
	m_error = 0;
	ldc_writeRegister(LDC_ENABLE, 0x00, m_ldc_address); // Power off device
	ldc_writeRegister(LDC_ENABLE, 0x01, m_ldc_address); // Power on device
	mcu_delay(2000);
	ldc_writeRegister(LDC_ATIME, INTEGRATION_TIMING, m_ldc_address); // Set integration timing to ca. 400ms
	ldc_writeRegister(LDC_CONTROL, ANALOG_GAIN, m_ldc_address); // Analog gain 16x
	ldc_writeRegister(LDC_ENABLE, 0x03, m_ldc_address); // Enable ADC
}

/*
 * Get data from sensor
 */

//_______________________________________________________________________________________________________
void getADC(int16_t *data) {
  if (m_error) {
    data[0] = 0;
    data[1] = 0;
  }
  uint8_t ADC[4];

  int res = i2c_read(m_ldc_address, (5<<5)+LDC_DATA0LOW, ADC, 4); // prepare auto-increment transaction
  if (res) {debug_print(DBG_ERROR, "i2c_read fail\n");}

  data[0] = (ADC[1]<<8)+ADC[0]; // ADC0 value
  data[1] = (ADC[3]<<8)+ADC[2]; // ADC1 value
}

/*
 * Read and Write
 */

//_______________________________________________________________________________________________________
uint8_t ldc_readRegister(uint8_t addr, uint8_t i2c) {
  if (m_error) {return 0;}

  unsigned char LSB;
  int res = i2c_read(i2c , (1<<7)+addr, &LSB, 1); // prepare standard read transaction
  if (res) {debug_print(DBG_ERROR, "i2c_read fail\n");}
  return LSB;
}

//_______________________________________________________________________________________________________
void ldc_writeRegister(uint8_t addr, uint8_t data, uint8_t i2c) {
	int res = i2c_write(i2c, (1<<7)+addr, &data, 1); // prepare standard read transaction
	if(res) {
		debug_print(DBG_ERROR, "i2c_write fail\n");
	    m_error = 1;
	}
}
