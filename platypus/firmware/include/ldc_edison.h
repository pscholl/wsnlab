/*
* Author: Sebastian Boettcher
* 
* Light-To-Digital Converter interface (TSL25723FN)
*
*/

#ifndef ldc_edison_h
#define ldc_edison_h

#include "mraa.hpp"

#include <string>
#include <sstream>
#include <vector>

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


class ldc_edison {
 public:
  ldc_edison(int i2c_bus = 1, uint8_t i2c_addr = LDC_I2C_ADDRESS);
  ~ldc_edison();

  // set the I2C address of the device
  void setAddress(uint8_t i2c_addr = LDC_I2C_ADDRESS);
  // reset and initialize the device
  void init();

  // get the current values from the ADCs, in a vector [ADC0,ADC1]
  std::vector<uint16_t> getADC();

  // get device ID
  uint8_t getID();

  // calculate Lux from current channel data, according to datasheet
  float getLux();

 private:
  // single read from the specified register
  uint8_t readRegister(uint8_t reg, uint8_t i2c);
  uint8_t readRegister(uint8_t reg);
  //single write to the specified register
  void writeRegister(uint8_t reg, uint8_t data, uint8_t i2c);
  void writeRegister(uint8_t reg, uint8_t data);

  mraa::I2c* m_i2c;
  uint8_t m_ldc_address;

};

#endif // ldc_edison_h

