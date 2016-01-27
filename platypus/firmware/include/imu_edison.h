/*
* Author: Sebastian Boettcher
* IMU interface for the Platypus, using MPU 9150/9250/9255
* includes Bosch environmental sensor access
*
*/

#ifndef imu_edison_h
#define imu_edison_h

#include <vector>
#include <math.h>
#include <assert.h>

#include "mraa.hpp"

#include "./quaternion.h"


// Register names according to the datasheet.
// According to the InvenSense document
// "MPU-9150 Register Map and Descriptions Revision 4.0",
#define MAX_BUFFER_LENGTH           6

#define MPU_SELF_TEST_X        0x0D   // R/W
#define MPU_SELF_TEST_Y        0x0E   // R/W
//#define MPU_SELF_TEST_X        0x0F   // R/W
#define MPU_SELF_TEST_A        0x10   // R/W
#define MPU_SMPLRT_DIV         0x19   // R/W
#define MPU_CONFIG             0x1A   // R/W
#define MPU_GYRO_CONFIG        0x1B   // R/W
#define MPU_ACCEL_CONFIG       0x1C   // R/W
#define MPU_ACCEL_CONFIG_2     0x1D   // R/W
#define MPU_LP_ACCEL           0x1E   // R/W
#define MPU_MOT_THR            0x1F   // R/W
#define MPU_MOT_DUR            0x20   // R/W
#define MPU_ZRMOT_THR          0x21   // R/W
#define MPU_ZRMOT_DUR          0x22   // R/W
#define MPU_FIFO_EN            0x23   // R/W
#define MPU_I2C_MST_CTRL       0x24   // R/W
#define MPU_I2C_SLV0_ADDR      0x25   // R/W
#define MPU_I2C_SLV0_REG       0x26   // R/W
#define MPU_I2C_SLV0_CTRL      0x27   // R/W
#define MPU_I2C_SLV1_ADDR      0x28   // R/W
#define MPU_I2C_SLV1_REG       0x29   // R/W
#define MPU_I2C_SLV1_CTRL      0x2A   // R/W
#define MPU_I2C_SLV2_ADDR      0x2B   // R/W
#define MPU_I2C_SLV2_REG       0x2C   // R/W
#define MPU_I2C_SLV2_CTRL      0x2D   // R/W
#define MPU_I2C_SLV3_ADDR      0x2E   // R/W
#define MPU_I2C_SLV3_REG       0x2F   // R/W
#define MPU_I2C_SLV3_CTRL      0x30   // R/W
#define MPU_I2C_SLV4_ADDR      0x31   // R/W
#define MPU_I2C_SLV4_REG       0x32   // R/W
#define MPU_I2C_SLV4_DO        0x33   // R/W
#define MPU_I2C_SLV4_CTRL      0x34   // R/W
#define MPU_I2C_SLV4_DI        0x35   // R   0x42   // R
#define MPU_GYRO_XOUT_H        0x43   // R
#define MPU_GYRO_XOUT_L        0x44   // R
#define MPU_GYRO_YOUT_H        0x45   // R
#define MPU_GYRO_YOUT_L        0x46   // R
#define MPU_GYRO_ZOUT_H        0x47   // R
#define MPU_GYRO_ZOUT_L        0x48   // R
#define MPU_I2C_MST_STATUS     0x36   // R
#define MPU_INT_PIN_CFG        0x37   // R/W
#define MPU_INT_ENABLE         0x38   // R/W
#define MPU_INT_STATUS         0x3A   // R
#define MPU_ACCEL_XOUT_H       0x3B   // R
#define MPU_ACCEL_XOUT_L       0x3C   // R
#define MPU_ACCEL_YOUT_H       0x3D   // R
#define MPU_ACCEL_YOUT_L       0x3E   // R
#define MPU_ACCEL_ZOUT_H       0x3F   // R
#define MPU_ACCEL_ZOUT_L       0x40   // R
#define MPU_TEMP_OUT_H         0x41   // R
#define MPU_TEMP_OUT_L         0x42   // R
#define MPU_EXT_SENS_DATA_00   0x49   // R
#define MPU_EXT_SENS_DATA_01   0x4A   // R
#define MPU_EXT_SENS_DATA_02   0x4B   // R
#define MPU_EXT_SENS_DATA_03   0x4C   // R
#define MPU_EXT_SENS_DATA_04   0x4D   // R
#define MPU_EXT_SENS_DATA_05   0x4E   // R
#define MPU_EXT_SENS_DATA_06   0x4F   // R
#define MPU_EXT_SENS_DATA_07   0x50   // R
#define MPU_EXT_SENS_DATA_08   0x51   // R
#define MPU_EXT_SENS_DATA_09   0x52   // R
#define MPU_EXT_SENS_DATA_10   0x53   // R
#define MPU_EXT_SENS_DATA_11   0x54   // R
#define MPU_EXT_SENS_DATA_12   0x55   // R
#define MPU_EXT_SENS_DATA_13   0x56   // R
#define MPU_EXT_SENS_DATA_14   0x57   // R
#define MPU_EXT_SENS_DATA_15   0x58   // R
#define MPU_EXT_SENS_DATA_16   0x59   // R
#define MPU_EXT_SENS_DATA_17   0x5A   // R
#define MPU_EXT_SENS_DATA_18   0x5B   // R
#define MPU_EXT_SENS_DATA_19   0x5C   // R
#define MPU_EXT_SENS_DATA_20   0x5D   // R
#define MPU_EXT_SENS_DATA_21   0x5E   // R
#define MPU_EXT_SENS_DATA_22   0x5F   // R
#define MPU_EXT_SENS_DATA_23   0x60   // R
#define MPU_MOT_DETECT_STATUS  0x61   // R
#define MPU_I2C_SLV0_DO        0x63   // R/W
#define MPU_I2C_SLV1_DO        0x64   // R/W
#define MPU_I2C_SLV2_DO        0x65   // R/W
#define MPU_I2C_SLV3_DO        0x66   // R/W
#define MPU_I2C_MST_DELAY_CTRL 0x67   // R/W
#define MPU_SIGNAL_PATH_RESET  0x68   // R/W
#define MPU_MOT_DETECT_CTRL    0x69   // R/W
#define MPU_USER_CTRL          0x6A   // R/W
#define MPU_PWR_MGMT_1         0x6B   // R/W
#define MPU_PWR_MGMT_2         0x6C   // R/W
#define MPU_FIFO_COUNTH        0x72   // R/W
#define MPU_FIFO_COUNTL        0x73   // R/W
#define MPU_FIFO_R_W           0x74   // R/W
#define MPU_WHO_AM_I           0x75   // R

//Magnetometer Registers
#define COMPASS_WHO_AM_I 0x00
#define COMPASS_INFO     0x01
#define COMPASS_ST1      0x02
#define COMPASS_XOUT_L   0x03
#define COMPASS_XOUT_H   0x04
#define COMPASS_YOUT_L   0x05
#define COMPASS_YOUT_H   0x06
#define COMPASS_ZOUT_L   0x07
#define COMPASS_ZOUT_H   0x08
#define COMPASS_ST2      0x09
#define COMPASS_CNTL     0x0A
#define COMPASS_ASTC     0x0C
#define COMPASS_I2CDIS   0x0F
#define COMPASS_ASAX     0x10
#define COMPASS_ASAY     0x11
#define COMPASS_ASAZ     0x12


// I2C address 0x69 could be 0x68 depends on your wiring.
#define MPU_I2C_ADDR           0x68
#define COMPASS_I2C_ADDR       0x0C
#define BME_I2C_ADDR           0x77


// Bosch BME280 Environmental Sensor Registers
#define BME_CALIB00     0x88
#define BME_CALIB25     0xA1
#define BME_ID          0xD0
#define BME_RESET       0xE0
#define BME_CALIB26     0xE1
#define BME_CALIB41     0xF0
#define BME_CTRL_HUM    0xF2
#define BME_STATUS      0xF3
#define BME_CTRL_MEAS   0xF4
#define BME_CONFIG      0xF5
#define BME_PRESS_MSB   0xF7
#define BME_PRESS_LSB   0xF8
#define BME_PRESS_XLSB  0xF9
#define BME_TEMP_MSB    0xFA
#define BME_TEMP_LSB    0xFB
#define BME_TEMP_XLSB   0xFC
#define BME_HUM_MSB     0xFD
#define BME_HUM_LSB     0xFE

struct BME_calibration {
  uint16_t dig_T1;
  int16_t  dig_T2;
  int16_t  dig_T3;

  uint16_t dig_P1;
  int16_t  dig_P2;
  int16_t  dig_P3;
  int16_t  dig_P4;
  int16_t  dig_P5;  
  int16_t  dig_P6;  
  int16_t  dig_P7; 
  int16_t  dig_P8;
  int16_t  dig_P9;

  uint8_t  dig_H1;
  int16_t  dig_H2;
  uint8_t  dig_H3;
  int16_t  dig_H4;
  int16_t  dig_H5;
  int8_t  dig_H6;
};

// Gyro Full-Scale range select
// 0: +-250 deg/s
// 1: +-500 deg/s
// 2: +-1000 deg/s
// 3: +-2000 deg/s
#define GFS_SEL 0

// Accel Full-Scale range select
// 0: +-2 g
// 1: +-4 g
// 2: +-8 g
// 3: +-16 g
#define AFS_SEL 1


class imu_edison {
 public:
  // constructor, creates a mraa I2C object
  imu_edison(int i2c_bus = 1, uint8_t i2c_addr = MPU_I2C_ADDR, bool init_env = false);
  // destructor, currently does nothing
  ~imu_edison();

  // sets the imu into sleep mode (true) or wakes it up (false)
  void sleep(bool sleep);

  // initializes the IMU
  void setupIMU();

  // get the IDs of the attached devices
  inline int getID() {return m_ID;}
  inline int getMagID() {return m_ID_mag;}
  inline int getEnvID() {return m_ID_env;}

  // reads the current accelerometer, gyroscope and temperature values
  // returns a vector according to [ACCEL_X, ACCEL_Y, ACCEL_Z, GYRO_X, GYRO_Y, GYRO_Z, TEMP]
  // raw 16Bit values, no conversion
  std::vector<int16_t> readRawIMU();
  // reads and returns only the temperature
  int16_t readRawTemp();

  // normalizes and converts 16Bit raw data to 8Bit raw data, loss of precision!
  std::vector<int8_t> convertData_2to1(std::vector<int16_t> in);
  // expands and converts 8Bit raw data to 16Bit raw data
  std::vector<int16_t> convertData_1to2(std::vector<int8_t> in);

  // returns readable data from given 8Bit or 16Bit raw data
  // accel values in m/s^2, gyro values in deg/s, temperature in degrees Celsius
  // !! only to be used in conjunction with return values from readRawIMU() !!
  std::vector<float> toReadable(std::vector<int8_t> in);
  std::vector<float> toReadable(std::vector<int16_t> in);

  // convert raw values to readable, according to data sheet
  float accelToReadable(int16_t a);
  float gyroToReadable(int16_t g);
  float tempToReadable(int16_t t);

  // returns the Interrupt Status register as is
  uint8_t getIntStatus();

  // returns true if a FIFO/WOM interrupt has been issued
  bool hasFIFOInt(uint8_t intStatus);
  bool hasWOMInt(uint8_t intStatus);

  // resets the FIFO module
  void FIFOrst();
  // returns count of bytes written in FIFO
  int FIFOcnt();
  // returns a vector of values that were in the FIFO at time of call
  // i.e. will only return the number of values that FIFOcnt() gives at time of call
  // will return multiples of six, in order ACCEL XYZ, GYRO XYZ
  // returns empty vector if less than six values in FIFO
  std::vector<int16_t> readFIFO();

  // reads the external sensor data from the 24 data registers
  std::vector<uint8_t> readESData();

  // compensate BME adc values and return integers
  // via BME280 datasheet sec 4.2.3
  int32_t tfine(int32_t adc_T);
  int32_t compT(int32_t adc_T);
  uint32_t compP(int32_t adc_P, int32_t adc_T);
  uint32_t compH(int32_t adc_H, int32_t adc_T);

  // reads the EnvSens data, and returns compensated values
  void getEnvData(int32_t &comp_T, uint32_t &comp_P, uint32_t &comp_H);
  void getEnvData(float &comp_T, float &comp_P, float &comp_H);

  // reads the Compass data, and returns raw values
  void getCompassData(int16_t &mag_X, int16_t &mag_Y, int16_t &mag_Z);
  // reads the Compass data, and returns compensated values in [mGs]
  void getCompassData(float &mag_X, float &mag_Y, float &mag_Z);

  // filter update step for the madgwick IMU filter
  // data <ax,ay,az,wx,wy,wz> [m/s^2,deg/s]
  // dT [seconds]
  // error [deg/s]
  void MadgwickIMUFilterStep(std::vector<float> data, float dT, float error, quaternion<float> &q);
  // same as above, but with magnetometer measurements (accesses I2C bus!)
  void MadgwickFilterStep(std::vector<float> data, float dT, float error, quaternion<float> &q);

 private:
  // initialize environmental sensor
  void initENV();

  // reads the calibration data for the BME device
  void getENVCalib();

  // initialize internal compass
  void initCompass();

  // write one byte [data] to register [addr] at i2c address [i2c]
  void writeRegister(uint8_t addr, uint8_t data, uint8_t i2c);
  // read from register [addr] at i2c address [i2c]
  uint8_t readRegister(uint8_t addr, uint8_t i2c);

  // read two bytes from registers [addrH] and [addrL] at i2c address [i2c]
  // returns the 16bit 2's complement integer value put together via [addrH addrL]
  int16_t readRegister(uint8_t addrH, uint8_t addrL, uint8_t i2c);

  mraa::I2c* m_i2c;
  int m_i2c_bus;
  uint8_t m_mpu_address;
  // BME calibration data, in the order of Table 16 in the BME280 datasheet
  BME_calibration m_env_calib;
  bool m_init_env;

  float m_HCalib_X, m_HCalib_Y, m_HCalib_Z;

  int m_ID, m_ID_mag, m_ID_env;

};

#endif // imu_edison_h

