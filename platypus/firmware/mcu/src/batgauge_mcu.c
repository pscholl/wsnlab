/*
 * Author: Mareike Höchner, Roshna George, Sebastian Boettcher
 * Code for the Implementation of the battery gauge MAX 17043 on Edison
 *
 */

#include "mcu_api.h"
#include "mcu_errno.h"
#include <stdint.h>
#include <string.h>

#include "batgauge_mcu.h"

/*
 * Read, Write, Init
 */

//_______________________________________________________________________________________________________
int readRegister_bat(uint8_t addrL, uint8_t addrH, int readFunction) {
	uint8_t LSB;
	uint8_t MSB;

	int res = i2c_read(MAX17043_ADDRESS , addrL, &LSB, 1);
    if (res) { debug_print(DBG_ERROR, "i2c_read fail\n");}
	res = i2c_read(MAX17043_ADDRESS , addrH, &MSB, 1);
    if (res) { debug_print(DBG_ERROR, "i2c_read fail\n");}

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

	    case 6 : // get the Alert Flag Bit
	      regValue = LSB & 0x20;
	      break;

	    default:
	      regValue = 0;
	  }

	  return regValue;
}

//_______________________________________________________________________________________________________
void writeRegister_bat(uint8_t addr, int16_t data) {
	int res = i2c_write(MAX17043_ADDRESS, addr, (unsigned char*) &data, 2);
	if (res) { debug_print(DBG_ERROR, "i2c_write fail\n");}
}

//_______________________________________________________________________________________________________
void setupBatgauge() {
	  reset();
	  mcu_sleep(100);
	  quickStart();
	  debug_print(DBG_INFO, "[BAT] Setup done.\n");
}

//_______________________________________________________________________________________________________
void reset() {
	writeRegister_bat(COMMAND_REGISTER, 0x5400);
}

//_______________________________________________________________________________________________________
void quickStart() {
	writeRegister_bat(MODE_REGISTER, 0x0040);
}


/*
 * get data
 */

//_______________________________________________________________________________________________________
int getVCell() {
	int vCell = readRegister_bat(VCELL_REGISTER_LSB, VCELL_REGISTER_MSB, FUNC_VCELL);
	return vCell; //* 0.00125;
}

//_______________________________________________________________________________________________________
int getSoC() {
	return readRegister_bat(SOC_REGISTER_LSB, SOC_REGISTER_MSB, FUNC_SOC);
}

//_______________________________________________________________________________________________________
int getVersion() {
	int MAX17043Version = readRegister_bat(VERSION_REGISTER_LSB, VERSION_REGISTER_MSB, FUNC_VER);
	return MAX17043Version;
}

//_______________________________________________________________________________________________________
int getAlertThreshold() {
	int alertThreshold = readRegister_bat(CONFIG_REGISTER_LSB, CONFIG_REGISTER_MSB, FUNC_GETALERT);
	return alertThreshold;
}

//_______________________________________________________________________________________________________
_Bool getAlertStatus() {
  int alertStatus = readRegister_bat(CONFIG_REGISTER_LSB, CONFIG_REGISTER_MSB, FUNC_STATUS);

  if (alertStatus == 0x20) {
	debug_print(DBG_INFO, "[BAT] Threshold reached!.\n");
    return 1;
  } else {
    return 0;
  }
}


/*
 * set alert
 */

//_______________________________________________________________________________________________________
void setAlertThreshold(int threshold) {
  if (threshold > 32) {
    threshold = 32;
  }

  int threshold_2scomplement = 32 - threshold ; // Alert threshold is in 2s complement .i.e 11111 = 0%

  int ConfigReg = readRegister_bat(CONFIG_REGISTER_LSB, CONFIG_REGISTER_MSB, FUNC_SETALERT);

  // Alert Threshold is the last five bits of the config register.
  // So here, the LSB is read first and the required alert threshold
  // is inserted into the last five bits.

  int writeData = (ConfigReg & 0xFFE0) | threshold_2scomplement;

  int writeData_swapped = (writeData >> 8) | (writeData << 8);

  writeRegister_bat(CONFIG_REGISTER_MSB, writeData_swapped);
  mcu_delay(1000);
}

