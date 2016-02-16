/*
* Author: Roshna George
* Code for the Implementation of the battery gauge MAX 17043 on Edison
*
*/


#include "mraa.h"

#include <stdio.h>
#include <unistd.h>

// Register Names


#define VCELL_REGISTER_MSB		0x02
#define VCELL_REGISTER_LSB		0x03

#define SOC_REGISTER_MSB		0x04
#define SOC_REGISTER_LSB		0x05

#define VERSION_REGISTER_MSB	0x08
#define VERSION_REGISTER_LSB	0x09

#define CONFIG_REGISTER_MSB		0x0C// The power up default value for config register is 97H
#define CONFIG_REGISTER_LSB		0x0D


#define MODE_REGISTER		0x06


#define COMMAND_REGISTER	0xFE


typedef unsigned char byte;
int MAX17043_ADDRESS = 0x36;

uint8_t rx_tx_buf[2];
int16_t regValue = 0;
int readFuntion = 0 ;
float decimal ;

// Function reading register Values

int MAX17043_readRegister (int addrL, int addrH){
  mraa_init();
  mraa_i2c_context i2c;
  i2c = mraa_i2c_init(1);                                // Battery gauge uses I2C1
  mraa_i2c_address(i2c, MAX17043_ADDRESS);
  byte LSB = mraa_i2c_read_byte_data(i2c,addrL);

  //printf("lower byte is:%02x\n\r ",LSB);    //Testing purposes


  mraa_i2c_address(i2c, MAX17043_ADDRESS);
  byte MSB = mraa_i2c_read_byte_data(i2c,addrH);

  //printf("Upper byte is:%02x\n\r ",MSB);     //Testing purposes

  switch (readFuntion) {

    case 1 : // get Vcell
        regValue = (MSB << 4) | (LSB >> 4);
        break;
    case 2 :// get SoC
        //decimal = LSB / 256.0;
        regValue = MSB;
        break;

    case 3 :// get battery Version
        regValue = (MSB << 8) | LSB;
        break;

    case 4 :    // get AlertThreshold

        regValue = 32 - (LSB & 0x1F);
        break;
    case 5 :    // reading the last five bits of the config register .i.e. the Alert threshold in 2s complement form

        regValue = (MSB << 8) | LSB;
        //printf("RegValue is:%04x\n\r ",regValue);
        break;

    case 6 :    // reading the last five bits of the config register .i.e. the Alert threshold in 2s complement form

        regValue = LSB & 0x20;
        //printf("RegValue is:%02x\n\r ",regValue);
        break;
  }


  //return (int16_t)((MSB<<8)+LSB);
  return regValue;
}

// Function writing values to register

int MAX17043_writeRegister(int addr,int16_t data ){
  mraa_i2c_context i2c;
  i2c = mraa_i2c_init(1);
  mraa_i2c_address(i2c, MAX17043_ADDRESS);
  //rx_tx_buf[0] = addr;
  //rx_tx_buf[1] = dataH;
  //rx_tx_buf[2] = data;
  mraa_i2c_write_word_data(i2c, data, addr);
  return 1;
}

void MAX17043_reset (){

    MAX17043_writeRegister(COMMAND_REGISTER, 0x5400);

}

void MAX17043_quickStart (){

    MAX17043_writeRegister(MODE_REGISTER, 0x0040);

}


float MAX17043_getVCell (){
    readFuntion = 1 ;
	int vCell = MAX17043_readRegister(VCELL_REGISTER_LSB,VCELL_REGISTER_MSB);

	//return (vCell, 0x000, 0xFFF, 0, 50000) / 10000.0;
	return vCell * 0.00125;
}

int MAX17043_getSoC (){

    readFuntion =2 ;
    return MAX17043_readRegister(SOC_REGISTER_LSB,SOC_REGISTER_MSB);
}

int MAX17043_getVersion (){

    readFuntion =3 ;
	int MAX17043Version = MAX17043_readRegister(VERSION_REGISTER_LSB,VERSION_REGISTER_MSB);
	return MAX17043Version ;
}

int MAX17043_getAlertThreshold (){

    readFuntion =4 ;
	int alertThreshold = MAX17043_readRegister(CONFIG_REGISTER_LSB,CONFIG_REGISTER_MSB);
	return alertThreshold ;
}

void MAX17043_setAlertThreshold (int alertThresholdValue){
    int threshold = alertThresholdValue ;
    if (threshold > 32){
        threshold = 32;
    }
    int threshold_2scomplement = 32 - threshold ; // Alert threshold is in 2s complement .i.e 11111 = 0%

    readFuntion =5 ;

    //printf("Threshold:%d %\n\r ",threshold);
    //printf("threshold_2scomplement:%d %\n\r ",threshold_2scomplement);

    int ConfigReg = MAX17043_readRegister(CONFIG_REGISTER_LSB,CONFIG_REGISTER_MSB);
    //printf("configReg:%02x \n\r ",ConfigReg);
    /**
        Alert Threshold is the last five bits of the config register. So here, the LSB is read first and the required alert threshold
    is inserted into the last five bits.
    **/

    int writeData = (ConfigReg & 0xFFE0) | threshold_2scomplement ;

    int writeData_swapped = (writeData  >>8) | (writeData <<8) ;


    //printf("writeData:%02x %\n\r ",writeData);

    //printf("writeData_swapped:%02x \n\r ",writeData_swapped);
	MAX17043_writeRegister(CONFIG_REGISTER_MSB, writeData_swapped);
    usleep(1000);
}

int MAX17043_getAlertStatus (){

    readFuntion =6 ;
	int alertStatus = MAX17043_readRegister(CONFIG_REGISTER_LSB,CONFIG_REGISTER_MSB);
	return alertStatus ;
}

int main(int argc, char** argv) {

    MAX17043_reset ();
    //printf("Battery Status:\t");
    sleep(1);

    MAX17043_quickStart ();

    float cellVoltage = MAX17043_getVCell ();
    printf("V: %1.5f\t ",cellVoltage);

    printf("C: %2d\t", MAX17043_getSoC());    

    int MAX17043Version = MAX17043_getVersion ();
    printf("MAX17043 v.%d \t ",MAX17043Version);

    int alertThreshold = MAX17043_getAlertThreshold ();
    printf("T: %d\t ",alertThreshold);

    //MAX17043_setAlertThreshold (20) ;

    //int alertThreshold_after = MAX17043_getAlertThreshold ();
    //printf("New Set Threshold:%d %\n\r ",alertThreshold_after);

    int alertStatus = MAX17043_getAlertStatus ();
    if (alertStatus == 0x20) {
         printf("1\n\r");
    }
    else {
        printf("0\n\r");
    }
    return 0;
}

