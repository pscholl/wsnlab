/*
 * Author: Roshna George, Sebastian Boettcher
 * Code for the Implementation of the battery gauge MAX 17043 on Edison
 *
 */


#include "./batgauge_edison.h"


//_______________________________________________________________________________________________________
batgauge_edison::batgauge_edison() {
  m_i2c = new mraa::I2c(1);
  m_i2c->address(MAX17043_ADDRESS);

  reset();
  sleep(1);
  quickStart();

  printf("[BAT] Battery gauge started.\n");
  fflush(stdout);
}

//_______________________________________________________________________________________________________
batgauge_edison::~batgauge_edison() {
}


/*
 * Read, Write, Init
 */

//_______________________________________________________________________________________________________
int batgauge_edison::readRegister(uint8_t addrL, uint8_t addrH, int readFunction) {

  uint8_t LSB = m_i2c->readReg(addrL);
  uint8_t MSB = m_i2c->readReg(addrH);

  int regValue;

  switch (readFunction) {
    case 1 : // get Vcell
      regValue = (MSB << 4) | (LSB >> 4);
      break;

    case 2 : // get SoC
      regValue = MSB;
      break;

    case 3 : // get battery Version
      regValue = (MSB << 8) | LSB;
      break;

    case 4 : // get AlertThreshold
      regValue = 32 - (LSB & 0x1F);
      break;

    case 5 : // reading the last five bits of the config register .i.e. the Alert threshold in 2s complement form
      regValue = (MSB << 8) | LSB;
      break;

    case 6 : // reading the last five bits of the config register .i.e. the Alert threshold in 2s complement form
      regValue = LSB & 0x20;
      break;

    default:
      regValue = 0;
  }

  return regValue;
}

//_______________________________________________________________________________________________________
void batgauge_edison::writeRegister(uint8_t addr, int16_t data) {
  m_i2c->writeWordReg(addr, data);
}

//_______________________________________________________________________________________________________
void batgauge_edison::reset() {
  writeRegister(COMMAND_REGISTER, 0x5400);
}

//_______________________________________________________________________________________________________
void batgauge_edison::quickStart() {
  writeRegister(MODE_REGISTER, 0x0040);
}



/*
 * get data
 */

//_______________________________________________________________________________________________________
float batgauge_edison::getVCell() {
	int vCell = readRegister(VCELL_REGISTER_LSB, VCELL_REGISTER_MSB, FUNC_VCELL);
	return vCell * 0.00125;
}

//_______________________________________________________________________________________________________
int batgauge_edison::getSoC() {
	return readRegister(SOC_REGISTER_LSB, SOC_REGISTER_MSB, FUNC_SOC);
}

//_______________________________________________________________________________________________________
int batgauge_edison::getVersion() {
	int MAX17043Version = readRegister(VERSION_REGISTER_LSB, VERSION_REGISTER_MSB, FUNC_VER);
	return MAX17043Version;
}

//_______________________________________________________________________________________________________
int batgauge_edison::getAlertThreshold() {
	int alertThreshold = readRegister(CONFIG_REGISTER_LSB, CONFIG_REGISTER_MSB, FUNC_GETALERT);
	return alertThreshold;
}

//_______________________________________________________________________________________________________
bool batgauge_edison::getAlertStatus() {
  int alertStatus = readRegister(CONFIG_REGISTER_LSB, CONFIG_REGISTER_MSB, FUNC_STATUS);

  if (alertStatus == 0x20) {
    printf("[BAT] Threshold reached!.\n");
    fflush(stdout);
    return true;
  } else {
    return false;
  }
}


/*
 * set alert
 */

//_______________________________________________________________________________________________________
void batgauge_edison::setAlertThreshold(int threshold) {
  if (threshold > 32) {
    threshold = 32;
  }

  int threshold_2scomplement = 32 - threshold ; // Alert threshold is in 2s complement .i.e 11111 = 0%

  int ConfigReg = readRegister(CONFIG_REGISTER_LSB, CONFIG_REGISTER_MSB, FUNC_SETALERT);

  // Alert Threshold is the last five bits of the config register.
  // So here, the LSB is read first and the required alert threshold
  // is inserted into the last five bits.

  int writeData = (ConfigReg & 0xFFE0) | threshold_2scomplement;

  int writeData_swapped = (writeData >> 8) | (writeData << 8);

	writeRegister(CONFIG_REGISTER_MSB, writeData_swapped);
  usleep(1000);
}

