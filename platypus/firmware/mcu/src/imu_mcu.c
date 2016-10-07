/*
* Author: Mareike Höchner, Sebastian Boettcher
* IMU interface for the Platypus, using MPU 9150/9250/9255
* includes Bosch environmental sensor access
*
*/

#include "mcu_api.h"
#include "mcu_errno.h"
#include <stdint.h>
#include <string.h>

#include "imu_mcu.h"

/*
 * Read and Write
 */

//_______________________________________________________________________________________________________
void writeRegister(uint8_t addr, uint8_t data, uint8_t i2c) {
	  int res = i2c_write(i2c, addr, &data, 1);
	  if(res) {debug_print(DBG_ERROR, "i2c_write fail\n");}
}

//_______________________________________________________________________________________________________
uint8_t readRegister(uint8_t addr, uint8_t i2c) {
    unsigned char LSB;
    int res = i2c_read(i2c , addr, &LSB, 1);

    if (res) {debug_print(DBG_ERROR, "i2c_read fail\n");}

	return LSB;
}

//_______________________________________________________________________________________________________
int16_t readRegister16(uint8_t addrH, uint8_t addrL, uint8_t i2c) {
	  unsigned char H;
	  int res = i2c_read(i2c , addrH, &H, 1);
	  if (res) {debug_print(DBG_ERROR, "i2c_read fail\n");}

	  unsigned char L;
	  res = i2c_read(i2c , addrL, &L, 1);
	  if (res) {debug_print(DBG_ERROR, "i2c_read fail\n");}

	  return (int16_t)((H<<8)+L);
}

//_______________________________________________________________________________________________________
void readBytesRegister(uint8_t addr, uint8_t *data, int len, uint8_t i2c) {
	  int res = i2c_read(i2c , addr, data, len);
	  if (res) {debug_print(DBG_ERROR, "i2c_read fail\n");}
}

/*
 * Initialization
 */

//_______________________________________________________________________________________________________
void stopIMU() {
	  sleep(1);
}

//_______________________________________________________________________________________________________
void sleep(_Bool sleep) {
  if (sleep) {
    // Set 'sleep' bit to 1 (bit 6) to go into sleep mode.
    writeRegister(MPU_PWR_MGMT_1, 0x40, m_mpu_address);
	debug_print(DBG_INFO, "[IMU] Sleep.\n");
  } else {
    // Clear the 'sleep' bit to start the sensor.
    writeRegister(MPU_PWR_MGMT_1, 0x00, m_mpu_address);
	debug_print(DBG_INFO, "[IMU] Wakeup.\n");
  }
}

//_______________________________________________________________________________________________________
void setupIMU(int samplerate_mpu, uint8_t i2c_addr, _Bool use_mpu_fifo, _Bool m_start_env, _Bool m_start_com) {
	m_mpu_address = i2c_addr;
	writeRegister(MPU_PWR_MGMT_1, 0x80, m_mpu_address); // reset device
	mcu_sleep(20);
	sleep(0);

	// BME init
	if(m_start_env) {
		initENV();
	}

	// Compass init
	if(m_start_com) {
		initCompass();
	}

	// MPU init
	writeRegister(MPU_SMPLRT_DIV, (1000/samplerate_mpu)-1, m_mpu_address); //set sample rate according to config (rate=1kHz/(1+div))
	writeRegister(MPU_CONFIG, 0x06, m_mpu_address); //set DLPF_CFG to lowest bandwith (5 Hz @ Fs=1kHz)
	writeRegister(MPU_GYRO_CONFIG, 0x00, m_mpu_address); //Gyro Config, fs_sel to +-250 deg/s
	writeRegister(MPU_ACCEL_CONFIG, 0x18, m_mpu_address); //Accel Config, fs_sel to +-16g
	writeRegister(MPU_ACCEL_CONFIG_2, 0x06, m_mpu_address); //Accel Config 2, set A_DLPF_CFG to lowest bandwith (5 Hz @ Fs=1kHz)
  	// Wake on FIFO Overflow
	if(use_mpu_fifo) {
		writeRegister(MPU_FIFO_EN, 0x78, m_mpu_address); //enable fifo buffer for accel XYZ, gyro XYZ
		writeRegister(MPU_INT_PIN_CFG, 0xA0, m_mpu_address); //interrupt pin config
		writeRegister(MPU_INT_ENABLE, 0x50, m_mpu_address); //enable interrupt for FIFO Overflow and WoM
		writeRegister(MPU_USER_CTRL, 0x64, m_mpu_address); //enable master i2c mode, enable FIFO, reset FIFO
	// Wake on RAW data ready
	} else {
		writeRegister(MPU_INT_PIN_CFG, 0xA0, m_mpu_address); //interrupt pin config
		writeRegister(MPU_INT_ENABLE, 0x41, m_mpu_address); //enable interrupt for Raw Sensor Data ready and WoM
		writeRegister(MPU_USER_CTRL, 0x20, m_mpu_address); //enable master i2c mode
	}
	writeRegister(MPU_MOT_THR, 0x80, m_mpu_address); //WoM threshold
	writeRegister(MPU_MOT_DETECT_CTRL, 0xC0, m_mpu_address); //WoM enable

	debug_print(DBG_INFO, "[IMU] Setup done.\n");
}

//_______________________________________________________________________________________________________
void initCompass() {
  writeRegister(MPU_USER_CTRL, 0x07, m_mpu_address); // reset FIFO, i2c, Signal
  writeRegister(MPU_INT_PIN_CFG, 0x02, m_mpu_address); // enable i2c master bypass

  writeRegister(COMPASS_CNTL, 0x00, COMPASS_I2C_ADDR); // Power down
  mcu_delay(1000);
  writeRegister(COMPASS_CNTL, 0x0F, COMPASS_I2C_ADDR); // Fuse ROM mode
  mcu_delay(1000);
//  m_HCalib_X = (float)(readRegister(COMPASS_ASAX, COMPASS_I2C_ADDR) - 128)/256.0 + 1.0;
//  m_HCalib_Y = (float)(readRegister(COMPASS_ASAY, COMPASS_I2C_ADDR) - 128)/256.0 + 1.0;
//  m_HCalib_Z = (float)(readRegister(COMPASS_ASAZ, COMPASS_I2C_ADDR) - 128)/256.0 + 1.0;
  writeRegister(COMPASS_CNTL, 0x00, COMPASS_I2C_ADDR); // Power down
  mcu_delay(1000);
  writeRegister(COMPASS_CNTL, 0x16, COMPASS_I2C_ADDR); // continuous 16bit measurement @ 100Hz
  mcu_delay(1000);

  m_ID_mag = readRegister(COMPASS_WHO_AM_I, COMPASS_I2C_ADDR);

  writeRegister(MPU_INT_PIN_CFG, 0x00, m_mpu_address); // disable i2c master bypass
  writeRegister(MPU_USER_CTRL, 0x20, m_mpu_address); //enable master i2c mode

  writeRegister(MPU_I2C_SLV1_ADDR, (0x80) | COMPASS_I2C_ADDR, m_mpu_address); // i2c address of compass; read operation
  writeRegister(MPU_I2C_SLV1_REG, 0x03, m_mpu_address); // register address of first data value
  writeRegister(MPU_I2C_SLV1_CTRL, 0x87, m_mpu_address); // enable slave 1; read 7 bytes per transaction

  debug_print(DBG_INFO, "[IMU] Compass init.\n");
}

//_______________________________________________________________________________________________________
void initENV() {
	  writeRegister(MPU_USER_CTRL, 0x07, m_mpu_address); // reset FIFO, i2c, Signal
	  writeRegister(MPU_INT_PIN_CFG, 0x02, m_mpu_address); // enable i2c master bypass

	  writeRegister(BME_RESET, 0xB6, BME_I2C_ADDR); // reset BME
	  mcu_sleep(20);
	  getENVCalib();

	  writeRegister(BME_CTRL_HUM, 0x01, BME_I2C_ADDR); // enable humidity sampling x1
	  writeRegister(BME_CONFIG, 0xA0, BME_I2C_ADDR); // 1s standby; IIR off
	  writeRegister(BME_CTRL_MEAS, 0x27, BME_I2C_ADDR); // enable temperature and pressure sampling x1; enable normal mode

	  writeRegister(MPU_INT_PIN_CFG, 0x00, m_mpu_address); // disable i2c master bypass
	  writeRegister(MPU_USER_CTRL, 0x20, m_mpu_address); //enable master i2c mode

	  writeRegister(MPU_I2C_SLV0_ADDR, (0x80) | BME_I2C_ADDR, m_mpu_address); // i2c address of env sens; read operation
	  writeRegister(MPU_I2C_SLV0_REG, 0xF7, m_mpu_address); // register address of first data value
	  writeRegister(MPU_I2C_SLV4_CTRL, 0x18, m_mpu_address); // set ext sens delay to once every 25 samples (1/s @ 25Hz)
	  writeRegister(MPU_I2C_MST_DELAY_CTRL, 0x81, m_mpu_address); // enable ext sens data shadowing delay; enable ext sens sample delay
	  writeRegister(MPU_I2C_SLV0_CTRL, 0x88, m_mpu_address); // enable slave 0; read 8 bytes per transaction

	  debug_print(DBG_INFO, "[IMU] ExtSens init.\n");
}

//_______________________________________________________________________________________________________
void getENVCalib() {
	  uint8_t calib_1[26];
	  uint8_t calib_2[16];

	  int res = i2c_read(BME_I2C_ADDR, BME_CALIB00, calib_1, 26);
	  if (res) { debug_print(DBG_ERROR, "i2c_read fail");}
	  res = i2c_read(BME_I2C_ADDR, BME_CALIB26, calib_2, 16);
	  if (res) { debug_print(DBG_ERROR, "i2c_read fail");}

	  m_env_calib.dig_T1 = (uint16_t) ((calib_1[1]<<8) | calib_1[0]);
	  m_env_calib.dig_T2 = (int16_t) ((calib_1[3]<<8) | calib_1[2]);
	  m_env_calib.dig_T3 = (int16_t) ((calib_1[5]<<8) | calib_1[4]);

	  m_env_calib.dig_P1 = (uint16_t) ((calib_1[7]<<8) | calib_1[6]);
	  m_env_calib.dig_P2 = (int16_t) ((calib_1[9]<<8) | calib_1[8]);
	  m_env_calib.dig_P3 = (int16_t) ((calib_1[11]<<8) | calib_1[10]);
	  m_env_calib.dig_P4 = (int16_t) ((calib_1[13]<<8) | calib_1[12]);
	  m_env_calib.dig_P5 = (int16_t) ((calib_1[15]<<8) | calib_1[14]);
	  m_env_calib.dig_P6 = (int16_t) ((calib_1[17]<<8) | calib_1[16]);
	  m_env_calib.dig_P7 = (int16_t) ((calib_1[19]<<8) | calib_1[18]);
	  m_env_calib.dig_P8 = (int16_t) ((calib_1[21]<<8) | calib_1[20]);
	  m_env_calib.dig_P9 = (int16_t) ((calib_1[23]<<8) | calib_1[22]);

	  m_env_calib.dig_H1 = (uint8_t) (calib_1[25]);
	  m_env_calib.dig_H2 = (int16_t) ((calib_2[1]<<8) | calib_2[0]);
	  m_env_calib.dig_H3 = (uint8_t) (calib_2[2]);
	  m_env_calib.dig_H4 = (int16_t) ((calib_2[3]<<4) | (calib_2[4] & 0x0F));
	  m_env_calib.dig_H5 = (int16_t) (((calib_2[4] & 0xF0)>>4) | (calib_2[5]<<4));
	  m_env_calib.dig_H6 = (int8_t) (calib_2[6]);
}

/*
 * Get data from sensor
 */

//_______________________________________________________________________________________________________
void readRawIMU(int16_t *data) {
  // read accelerometer data
  data[0] = readRegister16(MPU_ACCEL_XOUT_H, MPU_ACCEL_XOUT_L, m_mpu_address);
  data[1] = readRegister16(MPU_ACCEL_YOUT_H, MPU_ACCEL_YOUT_L, m_mpu_address);
  data[2] = readRegister16(MPU_ACCEL_ZOUT_H, MPU_ACCEL_ZOUT_L, m_mpu_address);
  // read gyroscope data
  data[3] = readRegister16(MPU_GYRO_XOUT_H, MPU_GYRO_XOUT_L, m_mpu_address);
  data[4] = readRegister16(MPU_GYRO_YOUT_H, MPU_GYRO_YOUT_L, m_mpu_address);
  data[5] = readRegister16(MPU_GYRO_ZOUT_H, MPU_GYRO_ZOUT_L, m_mpu_address);
  // read temperature sensor data
//  data[6] = readRegister16(MPU_TEMP_OUT_H, MPU_TEMP_OUT_L, m_mpu_address);
}

//_______________________________________________________________________________________________________
int16_t readRawTemp() {
  return readRegister16(MPU_TEMP_OUT_H, MPU_TEMP_OUT_L, m_mpu_address);
}



/*
 * Interrupt handling
 */

//_______________________________________________________________________________________________________
uint8_t getIntStatus() {
	return readRegister(MPU_INT_STATUS, m_mpu_address);
}

//_______________________________________________________________________________________________________
_Bool hasFIFOInt(uint8_t intStatus) {
  return (intStatus & (1<<4));
}
//_______________________________________________________________________________________________________
_Bool hasWOMInt(uint8_t intStatus) {
  return (intStatus & (1<<6));
}
//_______________________________________________________________________________________________________
_Bool hasRawRdyInt(uint8_t intStatus) {
  return (intStatus & 1);
}



/*
 * FIFO handling
 */

//_______________________________________________________________________________________________________
void FIFOrst() {
  writeRegister(MPU_USER_CTRL, 0x64, m_mpu_address); //enable master i2c mode, enable FIFO, reset FIFO
  debug_print(DBG_INFO, "[IMU] FIFO was reset.\n");
}

//_______________________________________________________________________________________________________
int FIFOcnt() {
  return readRegister16(MPU_FIFO_COUNTH, MPU_FIFO_COUNTL, m_mpu_address);
}

//_______________________________________________________________________________________________________
int readFIFO(int16_t *data) {
  int cnt = FIFOcnt();
  int i, j;
  for (i = 0; i < cnt/12; ++i) {
    for (j = 0; j < 6; ++j) {
      // read from FIFO two times, first High then Low part, and save combined int16
    	data[(i*6)+j] = readRegister16(MPU_FIFO_R_W, MPU_FIFO_R_W, m_mpu_address);
    }
  }
  return (cnt/12)*12;
}


/*
 * EnvSens handling
 */

//_______________________________________________________________________________________________________
void readESData(uint8_t *data) {
	readBytesRegister(MPU_EXT_SENS_DATA_00, data, 24, m_mpu_address);
}

//_______________________________________________________________________________________________________
int32_t tfine(int32_t adc_T) {
  int32_t var1, var2, t_fine;
  var1  = ((((adc_T >> 3) - ((int32_t)m_env_calib.dig_T1 << 1))) * ((int32_t)m_env_calib.dig_T2)) >> 11;
  var2  = (((((adc_T >> 4) - ((int32_t)m_env_calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)m_env_calib.dig_T1))) >> 12) * ((int32_t)m_env_calib.dig_T3)) >> 14;
  t_fine = var1 + var2;
  return t_fine;
}

//_______________________________________________________________________________________________________
int32_t compT(int32_t adc_T) {
  int32_t t_fine, T;
  t_fine = tfine(adc_T);
  T = (t_fine * 5 + 128) >> 8;
  return T;
}

//_______________________________________________________________________________________________________
uint32_t compP(int32_t adc_P, int32_t adc_T) {
  int32_t t_fine = tfine(adc_T);
  int32_t var1, var2;
  uint32_t p;
  var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
  var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)m_env_calib.dig_P6);
  var2 = var2 + ((var1*((int32_t)m_env_calib.dig_P5))<<1);
  var2 = (var2>>2)+(((int32_t)m_env_calib.dig_P4)<<16);
  var1 = (((m_env_calib.dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)m_env_calib.dig_P2) * var1)>>1))>>18;
  var1 =((((32768+var1))*((int32_t)m_env_calib.dig_P1))>>15);
  if (var1 == 0) {
	  return 0; // avoid exception caused by division by zero
  }
  p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
  if (p < 0x80000000) {
	  p = (p << 1) / ((uint32_t)var1);
  } else {
	  p = (p / (uint32_t)var1) * 2;
  }
  var1 = (((int32_t)m_env_calib.dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
  var2 = (((int32_t)(p>>2)) * ((int32_t)m_env_calib.dig_P8))>>13;
  p = (uint32_t)((int32_t)p + ((var1 + var2 + m_env_calib.dig_P7) >> 4));
  return p;
}

//_______________________________________________________________________________________________________
uint32_t compH(int32_t adc_H, int32_t adc_T) {
  int32_t t_fine = tfine(adc_T);
  int32_t v_x1_u32r;
  v_x1_u32r = (t_fine - ((int32_t)76800));
  v_x1_u32r = (((((adc_H << 14) - (((int32_t)m_env_calib.dig_H4) << 20) - (((int32_t)m_env_calib.dig_H5) * v_x1_u32r)) +
    ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)m_env_calib.dig_H6)) >> 10) * (((v_x1_u32r *
    ((int32_t)m_env_calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
    ((int32_t)m_env_calib.dig_H2) + 8192) >> 14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)m_env_calib.dig_H1)) >> 4));
  if(v_x1_u32r < 0) v_x1_u32r = 0;
  if(v_x1_u32r > 419430400) v_x1_u32r = 419430400;
  return (uint32_t)(v_x1_u32r >> 12);
}

//_______________________________________________________________________________________________________
void getEnvData(int32_t *comp_T, uint32_t *comp_P, uint32_t *comp_H) {
	uint8_t es_data_raw[24];
	readESData(es_data_raw);
	int32_t adc_T;
	int32_t adc_P;
	int32_t adc_H;

	adc_T  = (uint32_t)es_data_raw[5] >> 4;
	adc_T |= (uint32_t)es_data_raw[4] << 4;
	adc_T |= (uint32_t)es_data_raw[3] << 12;

	adc_P  = (uint32_t)es_data_raw[2] >> 4;
	adc_P |= (uint32_t)es_data_raw[1] << 4;
	adc_P |= (uint32_t)es_data_raw[0] << 12;

	adc_H = es_data_raw[7];
	adc_H |= (uint32_t)es_data_raw[6] << 8;

	*comp_T = compT(adc_T);
	*comp_P = compP(adc_P, adc_T);
	*comp_H = compH(adc_H, adc_T);
}

/*
 * Compass data
 */

//_______________________________________________________________________________________________________
void getCompassData(int16_t *mag_X, int16_t *mag_Y, int16_t *mag_Z) {
  uint8_t es_data_raw[24];
  readESData(es_data_raw);
  if (!(es_data_raw[14] & 0x08)) { // Check if magnetic sensor overflow set
	  *mag_X = (int16_t)(((int16_t)es_data_raw[9] << 8) | es_data_raw[8]);
	  *mag_Y = (int16_t)(((int16_t)es_data_raw[11] << 8) | es_data_raw[10]);
	  *mag_Z = (int16_t)(((int16_t)es_data_raw[13] << 8) | es_data_raw[12]);
  }
}
