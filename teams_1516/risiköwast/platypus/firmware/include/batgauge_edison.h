/*
 * Author: Roshna George, Sebastian Boettcher
 * Code for the Implementation of the battery gauge MAX 17043 on Edison
 *
 */

#ifndef batgauge_edison_h
#define batgauge_edison_h

#include <stdio.h>
#include <unistd.h>

#include "mraa.hpp"



// Register Names
#define VCELL_REGISTER_MSB  0x02
#define VCELL_REGISTER_LSB  0x03
#define SOC_REGISTER_MSB  0x04
#define SOC_REGISTER_LSB  0x05
#define VERSION_REGISTER_MSB  0x08
#define VERSION_REGISTER_LSB  0x09
#define CONFIG_REGISTER_MSB  0x0C// The power up default value for config register is 97H
#define CONFIG_REGISTER_LSB  0x0D
#define MODE_REGISTER  0x06
#define COMMAND_REGISTER  0xFE

#define MAX17043_ADDRESS  0x36

#define FUNC_VCELL  1
#define FUNC_SOC  2
#define FUNC_VER  3
#define FUNC_GETALERT  4
#define FUNC_SETALERT  5
#define FUNC_STATUS  6


class batgauge_edison {
 public:
  batgauge_edison(int i2c_bus = 1);
  ~batgauge_edison();

  void reset();
  void quickStart();

  float getVCell();
  int getSoC();
  int getVersion();

  bool getAlertStatus();

  int getAlertThreshold();
  void setAlertThreshold (int threshold);


 private:
  int readRegister(uint8_t addrL, uint8_t addrH, int readFunction);
  void writeRegister(uint8_t addr, int16_t data);

  mraa::I2c* m_i2c;

};

#endif // batgauge_edison_h
