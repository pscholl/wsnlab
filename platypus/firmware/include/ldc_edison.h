/*
* Author: Sebastian Boettcher
* 
* Light-To-Digital Converter interface (TSL2584TSV)
*
*/

#ifndef ldc_edison_h
#define ldc_edison_h

#include "mraa.hpp"

#include <string>
#include <sstream>
#include <vector>

#define LDC_CONTROL     0x00
#define LDC_TIMING      0x01
#define LDC_INTERRUPT   0x02
#define LDC_THLLOW      0x03
#define LDC_THLHIGH     0x04
#define LDC_THHLOW      0x05
#define LDC_THHHIGH     0x06
#define LDC_ANALOG      0x07
#define LDC_ID          0x12
#define LDC_DATA0LOW    0x14
#define LDC_DATA0HIGH   0x15
#define LDC_DATA1LOW    0x16
#define LDC_DATA1HIGH   0x17
#define LDC_TIMERLOW    0x18
#define LDC_TIMERHIGH   0x19
#define LDC_ID2         0x1E

#define LDC_I2C_ADDRESS 0x29


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

  // get a version string of the device as "ADDRESS:PARTNO|REVNO|ID2"
  std::string getVersion();

 private:
  // single read from the specified register
  uint8_t readRegister(uint8_t addr, uint8_t i2c);
  uint8_t readRegister(uint8_t addr);
  //single write to the specified register
  void writeRegister(uint8_t addr, uint8_t data, uint8_t i2c);
  void writeRegister(uint8_t addr, uint8_t data);

  mraa::I2c* m_i2c;
  uint8_t m_ldc_address;
  bool m_error;

};

#endif // ldc_edison_h

