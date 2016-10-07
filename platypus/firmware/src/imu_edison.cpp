/*
* Author: Sebastian Boettcher
* IMU interface for the Platypus, using MPU 9150/9250/9255
* includes Bosch environmental sensor access
*
*/

#include "./imu_edison.h"



//_______________________________________________________________________________________________________
imu_edison::imu_edison(int i2c_bus, uint8_t i2c_addr, bool init_env, bool init_sens)
 : m_i2c_bus(i2c_bus), m_mpu_address(i2c_addr), m_init_env(init_env),
 m_ID(-1), m_ID_mag(-1), m_ID_env(-1)
{
  if (init_sens) {
    m_i2c = new mraa::I2c(m_i2c_bus);
    m_i2c->address(m_mpu_address);
  }
}


//_______________________________________________________________________________________________________
imu_edison::~imu_edison() {
  sleep(true);
}


/*
 * Read and Write
 */


//_______________________________________________________________________________________________________
void imu_edison::writeRegister(uint8_t addr, uint8_t data, uint8_t i2c) {
  m_i2c->address(i2c);

  uint8_t rx_tx_buf[2];
  rx_tx_buf[0] = addr;
  rx_tx_buf[1] = data;
  m_i2c->write(rx_tx_buf, 2);
}

//_______________________________________________________________________________________________________
uint8_t imu_edison::readRegister(uint8_t addr, uint8_t i2c) {
  m_i2c->address(i2c);

  uint8_t data = 0;

  try {
    data = m_i2c->readReg(addr);
  } catch (std::invalid_argument& e) {}

  return data;
}

//_______________________________________________________________________________________________________
int16_t imu_edison::readRegister(uint8_t addrH, uint8_t addrL, uint8_t i2c) {
  m_i2c->address(i2c);

  uint8_t H = 0, L = 0;

  try {
    H = m_i2c->readReg(addrH);
    L = m_i2c->readReg(addrL);
  } catch (std::invalid_argument& e) {}

  //printf("Upper byte is:%d\n\r ",H);
  //printf("Lower byte is:%d\n\r ",L);

  return (int16_t)((H<<8)+L);
}



/*
 * Initialization
 */

//_______________________________________________________________________________________________________
void imu_edison::sleep(bool sleep) {
  if (sleep) {
    // Set 'sleep' bit to 1 (bit 6) to go into sleep mode.
    writeRegister(MPU_PWR_MGMT_1, 0x40, m_mpu_address);
    printf("[IMU] Sleep.\n");
    fflush(stdout);
  } else {
    // Clear the 'sleep' bit to start the sensor.
    writeRegister(MPU_PWR_MGMT_1, 0x00, m_mpu_address);
    printf("[IMU] Wakeup.\n");
    fflush(stdout);
  }
}


//_______________________________________________________________________________________________________
void imu_edison::setupIMU() {
  writeRegister(MPU_PWR_MGMT_1, 0x80, m_mpu_address); // reset device
  usleep(200000);
  sleep(false);

  // BME init
  if (m_init_env)
    initENV();

  initCompass();

  assert(GFS_SEL >= 0 && GFS_SEL <= 3);
  assert(AFS_SEL >= 0 && AFS_SEL <= 3);

  // MPU init
  writeRegister(MPU_SMPLRT_DIV, 0x27, m_mpu_address); //set sample rate to 25Hz (rate=1kHz/(1+div))
  writeRegister(MPU_CONFIG, 0x06, m_mpu_address); //set DLPF_CFG to lowest bandwith (5 Hz @ Fs=1kHz)
  writeRegister(MPU_GYRO_CONFIG, GFS_SEL << 3, m_mpu_address); //Gyro Config, fs_sel
  writeRegister(MPU_ACCEL_CONFIG, AFS_SEL << 3, m_mpu_address); //Accel Config, fs_sel
  writeRegister(MPU_ACCEL_CONFIG_2, 0x00, m_mpu_address); //Accel Config 2, set A_DLPF_CFG to highest bandwith (460 Hz @ Fs=1kHz)
  writeRegister(MPU_FIFO_EN, 0x78, m_mpu_address); //enable fifo buffer for accel XYZ, gyro XYZ
  writeRegister(MPU_INT_PIN_CFG, 0xA0, m_mpu_address); //interrupt pin config
  writeRegister(MPU_INT_ENABLE, 0x50, m_mpu_address); //enable interrupt for FIFO and WoM
  writeRegister(MPU_MOT_THR, 0x80, m_mpu_address); //WoM threshold
  writeRegister(MPU_MOT_DETECT_CTRL, 0xC0, m_mpu_address); //WoM enable
  writeRegister(MPU_USER_CTRL, 0x64, m_mpu_address); //enable master i2c mode, enable FIFO, reset FIFO

  m_ID = readRegister(MPU_WHO_AM_I, m_mpu_address);

  printf("[IMU] Setup done.\n");
  fflush(stdout);
}

//_______________________________________________________________________________________________________
void imu_edison::initCompass() {
  writeRegister(MPU_USER_CTRL, 0x07, m_mpu_address); // reset FIFO, i2c, Signal
  writeRegister(MPU_INT_PIN_CFG, 0x02, m_mpu_address); // enable i2c master bypass

  writeRegister(COMPASS_CNTL, 0x00, COMPASS_I2C_ADDR); // Power down
  usleep(1000);
  writeRegister(COMPASS_CNTL, 0x0F, COMPASS_I2C_ADDR); // Fuse ROM mode
  usleep(1000);
  m_HCalib_X = (float)(readRegister(COMPASS_ASAX, COMPASS_I2C_ADDR) - 128)/256.0 + 1.0;
  m_HCalib_Y = (float)(readRegister(COMPASS_ASAY, COMPASS_I2C_ADDR) - 128)/256.0 + 1.0;
  m_HCalib_Z = (float)(readRegister(COMPASS_ASAZ, COMPASS_I2C_ADDR) - 128)/256.0 + 1.0;
  writeRegister(COMPASS_CNTL, 0x00, COMPASS_I2C_ADDR); // Power down
  usleep(1000);
  writeRegister(COMPASS_CNTL, 0x16, COMPASS_I2C_ADDR); // continuous 16bit measurement @ 100Hz
  usleep(1000);

  m_ID_mag = readRegister(COMPASS_WHO_AM_I, COMPASS_I2C_ADDR);

  writeRegister(MPU_INT_PIN_CFG, 0x00, m_mpu_address); // disable i2c master bypass
  writeRegister(MPU_USER_CTRL, 0x20, m_mpu_address); //enable master i2c mode

  writeRegister(MPU_I2C_SLV1_ADDR, (0x80) | COMPASS_I2C_ADDR, m_mpu_address); // i2c address of compass; read operation
  writeRegister(MPU_I2C_SLV1_REG, 0x03, m_mpu_address); // register address of first data value
  writeRegister(MPU_I2C_SLV1_CTRL, 0x87, m_mpu_address); // enable slave 1; read 7 bytes per transaction

  printf("[IMU] Compass init.\n");
}

//_______________________________________________________________________________________________________
void imu_edison::initENV() {
  writeRegister(MPU_USER_CTRL, 0x07, m_mpu_address); // reset FIFO, i2c, Signal
  writeRegister(MPU_INT_PIN_CFG, 0x02, m_mpu_address); // enable i2c master bypass

  writeRegister(BME_RESET, 0xB6, BME_I2C_ADDR); // reset BME
  usleep(200000);
  getENVCalib();

  writeRegister(BME_CTRL_HUM, 0x01, BME_I2C_ADDR); // enable humidity sampling x1
  writeRegister(BME_CONFIG, 0xA0, BME_I2C_ADDR); // 1s standby; IIR off
  writeRegister(BME_CTRL_MEAS, 0x27, BME_I2C_ADDR); // enable temperature and pressure sampling x1; enable normal mode

  m_ID_env = readRegister(BME_ID, BME_I2C_ADDR);

  writeRegister(MPU_INT_PIN_CFG, 0x00, m_mpu_address); // disable i2c master bypass
  writeRegister(MPU_USER_CTRL, 0x20, m_mpu_address); //enable master i2c mode

  writeRegister(MPU_I2C_SLV0_ADDR, (0x80) | BME_I2C_ADDR, m_mpu_address); // i2c address of env sens; read operation
  writeRegister(MPU_I2C_SLV0_REG, 0xF7, m_mpu_address); // register address of first data value
  writeRegister(MPU_I2C_SLV4_CTRL, 0x18, m_mpu_address); // set ext sens delay to once every 25 samples (1/s @ 25Hz)
  writeRegister(MPU_I2C_MST_DELAY_CTRL, 0x81, m_mpu_address); // enable ext sens data shadowing delay; enable ext sens sample delay
  writeRegister(MPU_I2C_SLV0_CTRL, 0x88, m_mpu_address); // enable slave 0; read 8 bytes per transaction

  printf("[IMU] ExtSens init.\n");
}

//_______________________________________________________________________________________________________
void imu_edison::getENVCalib() {
  uint8_t calib_1[26];
  uint8_t calib_2[16];

  try {
    m_i2c->address(BME_I2C_ADDR);
    m_i2c->writeByte(BME_CALIB00);
    m_i2c->read(calib_1, 26);
    m_i2c->writeByte(BME_CALIB26);
    m_i2c->read(calib_2, 16);
  } catch (std::invalid_argument& e) {}

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
std::vector<int16_t> imu_edison::readRawIMU() {
  std::vector<int16_t> data;

  // read accelerometer data
  data.push_back(readRegister(MPU_ACCEL_XOUT_H, MPU_ACCEL_XOUT_L, m_mpu_address));
  data.push_back(readRegister(MPU_ACCEL_YOUT_H, MPU_ACCEL_YOUT_L, m_mpu_address));
  data.push_back(readRegister(MPU_ACCEL_ZOUT_H, MPU_ACCEL_ZOUT_L, m_mpu_address));
  // read gyroscope data
  data.push_back(readRegister(MPU_GYRO_XOUT_H, MPU_GYRO_XOUT_L, m_mpu_address));
  data.push_back(readRegister(MPU_GYRO_YOUT_H, MPU_GYRO_YOUT_L, m_mpu_address));
  data.push_back(readRegister(MPU_GYRO_ZOUT_H, MPU_GYRO_ZOUT_L, m_mpu_address));
  // read temperature sensor data
  data.push_back(readRegister(MPU_TEMP_OUT_H, MPU_TEMP_OUT_L, m_mpu_address));

  return data;
}

//_______________________________________________________________________________________________________
int16_t imu_edison::readRawTemp() {
  return readRegister(MPU_TEMP_OUT_H, MPU_TEMP_OUT_L, m_mpu_address);
}



/*
 * Data conversions
 */

//_______________________________________________________________________________________________________
std::vector<int8_t> imu_edison::convertData_2to1(std::vector<int16_t> in) {
  std::vector<int8_t> out;

  for (size_t i = 0; i < in.size(); ++i)
    out.push_back((int8_t) ((in[i] + 128) / 255));

  return out;
}

//_______________________________________________________________________________________________________
std::vector<int16_t> imu_edison::convertData_1to2(std::vector<int8_t> in) {
  std::vector<int16_t> out;

  for (size_t i = 0; i < in.size(); ++i)
    out.push_back((int16_t) ((in[i] * 255) - 128));

  return out;
}

//_______________________________________________________________________________________________________
std::vector<float> imu_edison::toReadable(std::vector<int8_t> in) {
  return toReadable(convertData_1to2(in));
}

//_______________________________________________________________________________________________________
std::vector<float> imu_edison::toReadable(std::vector<int16_t> in) {
  assert(in.size() == 7);

  std::vector<float> out;

  // convert accelerometer data according to MPU9250 data sheet
  out.push_back(accelToReadable(in[0]));
  out.push_back(accelToReadable(in[1]));
  out.push_back(accelToReadable(in[2]));
  // convert gyroscope data according to MPU9250 data sheet
  out.push_back(gyroToReadable(in[3]));
  out.push_back(gyroToReadable(in[4]));
  out.push_back(gyroToReadable(in[5]));
  // convert temperature sensor data according to MPU9250 data sheet
  out.push_back(tempToReadable(in[6]));

  return out;
}

//_______________________________________________________________________________________________________
float imu_edison::accelToReadable(int16_t a) {
  return (a / (16384.0 / pow(2, AFS_SEL)) * 9.807);
}
//_______________________________________________________________________________________________________
float imu_edison::gyroToReadable(int16_t g) {
  return (g / (131.0 / pow(2, GFS_SEL)));
}
//_______________________________________________________________________________________________________
float imu_edison::tempToReadable(int16_t t) {
  return (((t - 21) / 333.87) + 21);
}


/*
 * Interrupt handling
 */

//_______________________________________________________________________________________________________
uint8_t imu_edison::getIntStatus() {
  return readRegister(MPU_INT_STATUS, m_mpu_address);
}

//_______________________________________________________________________________________________________
bool imu_edison::hasFIFOInt(uint8_t intStatus) {
  return (intStatus & (1<<4));
}
//_______________________________________________________________________________________________________
bool imu_edison::hasWOMInt(uint8_t intStatus) {
  return (intStatus & (1<<6));
}



/*
 * FIFO handling
 */

//_______________________________________________________________________________________________________
void imu_edison::FIFOrst() {
  writeRegister(MPU_USER_CTRL, 0x64, m_mpu_address); //enable master i2c mode, enable FIFO, reset FIFO
  printf("[IMU] FIFO was reset.\n");
  fflush(stdout);
}


//_______________________________________________________________________________________________________
int imu_edison::FIFOcnt() {
  return readRegister(MPU_FIFO_COUNTH, MPU_FIFO_COUNTL, m_mpu_address);
}

//_______________________________________________________________________________________________________
std::vector<int16_t> imu_edison::readFIFO() {
  int cnt = FIFOcnt();
  std::vector<int16_t> data;

  for (int i = 0; i < cnt/12; ++i) {
    for (int j = 0; j < 6; ++j) {
      // read from FIFO two times, first High then Low part, and save combined int16
      data.push_back(readRegister(MPU_FIFO_R_W, MPU_FIFO_R_W, m_mpu_address));
    }
  }

  return data;
}


/*
 * EnvSens handling
 */

//_______________________________________________________________________________________________________
std::vector<uint8_t> imu_edison::readESData() {
  std::vector<uint8_t> data;
  uint8_t buffer[24];

  try {
    m_i2c->readBytesReg(MPU_EXT_SENS_DATA_00, buffer, 24);
  } catch (std::invalid_argument& e) {}

  data.assign(buffer, buffer + 24);
  return data;
}

//_______________________________________________________________________________________________________
int32_t imu_edison::tfine(int32_t adc_T) {
  int32_t var1, var2, t_fine;
  var1  = ((((adc_T >> 3) - ((int32_t)m_env_calib.dig_T1 << 1))) * ((int32_t)m_env_calib.dig_T2)) >> 11;
  var2  = (((((adc_T >> 4) - ((int32_t)m_env_calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)m_env_calib.dig_T1))) >> 12) * ((int32_t)m_env_calib.dig_T3)) >> 14;
  t_fine = var1 + var2;
  return t_fine;
}

//_______________________________________________________________________________________________________
int32_t imu_edison::compT(int32_t adc_T) {
  int32_t t_fine, T;
  t_fine = tfine(adc_T);
  T = (t_fine * 5 + 128) >> 8;
  return T;
}

//_______________________________________________________________________________________________________
uint32_t imu_edison::compP(int32_t adc_P, int32_t adc_T) {
  int32_t t_fine = tfine(adc_T);
  int64_t var1, var2, p;
  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)m_env_calib.dig_P6;
  var2 = var2 + ((var1*(int64_t)m_env_calib.dig_P5) << 17);
  var2 = var2 + (((int64_t)m_env_calib.dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)m_env_calib.dig_P3) >> 8) + ((var1 * (int64_t)m_env_calib.dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)m_env_calib.dig_P1) >> 33;
  if (var1 == 0)
    return 0;
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)m_env_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)m_env_calib.dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)m_env_calib.dig_P7) << 4);
  return (uint32_t)p;
}

//_______________________________________________________________________________________________________
uint32_t imu_edison::compH(int32_t adc_H, int32_t adc_T) {
  int32_t t_fine = tfine(adc_T);
  int32_t v_x1_u32r;
  v_x1_u32r = (t_fine - ((int32_t)76800));
  v_x1_u32r = (((((adc_H << 14) - (((int32_t)m_env_calib.dig_H4) << 20) - (((int32_t)m_env_calib.dig_H5) * v_x1_u32r)) +
    ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)m_env_calib.dig_H6)) >> 10) * (((v_x1_u32r *
    ((int32_t)m_env_calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
    ((int32_t)m_env_calib.dig_H2) + 8192) >> 14));
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)m_env_calib.dig_H1)) >> 4));
  v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
  v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
  return (uint32_t)(v_x1_u32r >> 12);
}

//_______________________________________________________________________________________________________
void imu_edison::getEnvData(int32_t &comp_T, uint32_t &comp_P, uint32_t &comp_H) {
  std::vector<uint8_t> es_data_raw = readESData();
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

  comp_T = compT(adc_T);
  comp_P = compP(adc_P, adc_T);
  comp_H = compH(adc_H, adc_T);
}

//_______________________________________________________________________________________________________
void imu_edison::getEnvData(float &comp_T, float &comp_P, float &comp_H) {
  int32_t T;
  uint32_t P, H;
  getEnvData(T, P, H);
  comp_T = T / 100.0; // in [DegC]
  comp_P = P / 25600.0; // in [hPa]
  comp_H = H / 1024.0; // in [%RH]
}


/*
 * Compass data
 */

//_______________________________________________________________________________________________________
void imu_edison::getCompassData(int16_t &mag_X, int16_t &mag_Y, int16_t &mag_Z) {
  std::vector<uint8_t> es_data_raw = readESData();
  if (!(es_data_raw[14] & 0x08)) { // Check if magnetic sensor overflow set
    mag_X = (int16_t)(((int16_t)es_data_raw[9] << 8) | es_data_raw[8]);
    mag_Y = (int16_t)(((int16_t)es_data_raw[11] << 8) | es_data_raw[10]);
    mag_Z = (int16_t)(((int16_t)es_data_raw[13] << 8) | es_data_raw[12]);
  }
}
//_______________________________________________________________________________________________________
void imu_edison::getCompassData(float &mag_X, float &mag_Y, float &mag_Z) {
  int16_t raw_x, raw_y, raw_z;
  getCompassData(raw_x, raw_y, raw_z);
  mag_X = (float) (raw_x * (10.0 * 4912.0 / 32760.0) * m_HCalib_X);
  mag_Y = (float) (raw_y * (10.0 * 4912.0 / 32760.0) * m_HCalib_Y);
  mag_Z = (float) (raw_z * (10.0 * 4912.0 / 32760.0) * m_HCalib_Z);
}



/*
 * gyro orientation filter
 */

// Implementation of Madgwick's IMU algorithm.
// See: http://www.x-io.co.uk/open-source-imu-and-ahrs-algorithms/
void imu_edison::MadgwickIMUFilterStep(std::vector<float> data, float dT, float error, quaternion<float> &q) {
  float norm;
  float s0, s1, s2, s3;
  float qDot1, qDot2, qDot3, qDot4;
  float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;

  float q0 = q.m_w;
  float q1 = q.m_x;
  float q2 = q.m_y;
  float q3 = q.m_z;

  // compute gyro measurement error
  float beta = sqrt(3.0/4.0) * (M_PI * (error / 180.0));

  // convert to correct unit
  float ax = data[0];
  float ay = data[1];
  float az = data[2];
  float gx = (M_PI * (data[3] / 180.0));
  float gy = (M_PI * (data[4] / 180.0));
  float gz = (M_PI * (data[5] / 180.0));

  // Rate of change of quaternion from gyroscope
  qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
  qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
  qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
  qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

  // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
  if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f)) && !((gx == 0.0f) && (gy == 0.0f) && (gz == 0.0f))) {

    // Normalise accelerometer measurement
    norm = sqrt(ax * ax + ay * ay + az * az);
    ax /= norm;
    ay /= norm;
    az /= norm;

    // Auxiliary variables to avoid repeated arithmetic
    _2q0 = 2.0f * q0;
    _2q1 = 2.0f * q1;
    _2q2 = 2.0f * q2;
    _2q3 = 2.0f * q3;
    _4q0 = 4.0f * q0;
    _4q1 = 4.0f * q1;
    _4q2 = 4.0f * q2;
    _8q1 = 8.0f * q1;
    _8q2 = 8.0f * q2;
    q0q0 = q0 * q0;
    q1q1 = q1 * q1;
    q2q2 = q2 * q2;
    q3q3 = q3 * q3;

    // Gradient decent algorithm corrective step
    s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
    s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
    s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
    s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;
    norm = sqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
    s0 /= norm;
    s1 /= norm;
    s2 /= norm;
    s3 /= norm;

    // Apply feedback step
    qDot1 -= beta * s0;
    qDot2 -= beta * s1;
    qDot3 -= beta * s2;
    qDot4 -= beta * s3;
  }

  // Integrate rate of change of quaternion to yield quaternion
  q0 += qDot1 * dT;
  q1 += qDot2 * dT;
  q2 += qDot3 * dT;
  q3 += qDot4 * dT;

  // Normalise quaternion
  norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
  q0 /= norm;
  q1 /= norm;
  q2 /= norm;
  q3 /= norm;

  q.set(q1, q2, q3, q0);
}


void imu_edison::MadgwickFilterStep(std::vector<float> data, float dT, float error, quaternion<float> &q) {
  float norm;
  float s0, s1, s2, s3;
  float qDot1, qDot2, qDot3, qDot4;
  float hx, hy;
  float mx, my, mz;
  float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2bz, _4bx, _4bz, _8bx, _8bz, _2q0, _2q1, _2q2, _2q3, q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;

  float q0 = q.m_w;
  float q1 = q.m_x;
  float q2 = q.m_y;
  float q3 = q.m_z;

  // compute gyro measurement error
  float beta = sqrt(3.0/4.0) * (M_PI * (error / 180.0));

  // convert to correct unit
  float ax = data[0];
  float ay = data[1];
  float az = data[2];
  float gx = (M_PI * (data[3] / 180.0));
  float gy = (M_PI * (data[4] / 180.0));
  float gz = (M_PI * (data[5] / 180.0));

  getCompassData(mx, my, mz);

  // Rate of change of quaternion from gyroscope
  qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
  qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
  qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
  qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

  // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
  if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))
   && !((gx == 0.0f) && (gy == 0.0f) && (gz == 0.0f))
   && !((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f))) {

    // Normalise accelerometer measurement
    norm = sqrt(ax * ax + ay * ay + az * az);
    ax /= norm;
    ay /= norm;
    az /= norm;

    // Normalise magnetometer measurement
    norm = sqrt(mx * mx + my * my + mz * mz);
    mx /= norm;
    my /= norm;
    mz /= norm;

    // Auxiliary variables to avoid repeated arithmetic
    _2q0mx = 2.0f * q0 * mx;
    _2q0my = 2.0f * q0 * my;
    _2q0mz = 2.0f * q0 * mz;
    _2q1mx = 2.0f * q1 * mx;
    _2q0 = 2.0f * q0;
    _2q1 = 2.0f * q1;
    _2q2 = 2.0f * q2;
    _2q3 = 2.0f * q3;
    q0q0 = q0 * q0;
    q0q1 = q0 * q1;
    q0q2 = q0 * q2;
    q0q3 = q0 * q3;
    q1q1 = q1 * q1;
    q1q2 = q1 * q2;
    q1q3 = q1 * q3;
    q2q2 = q2 * q2;
    q2q3 = q2 * q3;
    q3q3 = q3 * q3;

    // Reference direction of Earth's magnetic field
    hx = mx * q0q0 - _2q0my * q3 + _2q0mz * q2 + mx * q1q1 + _2q1 * my * q2 + _2q1 * mz * q3 - mx  * q2q2 - mx * q3q3;
    hy = _2q0mx * q3 + my * q0q0 - _2q0mz * q1 + _2q1mx * q2 - my * q1q1 + my * q2q2 + _2q2 * mz * q3 - my * q3q3;
    _2bx = sqrt(hx * hx + hy * hy);
    _2bz = -_2q0mx * q2 + _2q0my * q1 + mz * q0q0 + _2q1mx * q3 - mz * q1q1 + _2q2 * my * q3 - mz  * q2q2 + mz * q3q3;
    _4bx = 2.0f * _2bx;
    _4bz = 2.0f * _2bz;
    _8bx = 2.0f * _4bx;
    _8bz = 2.0f * _4bz;

    // Gradient decent algorithm corrective step
    s0= -_2q2*(2.0f*(q1q3 - q0q2) - ax) + _2q1*(2.0f*(q0q1 + q2q3) - ay) + -_4bz*q2*(_4bx*(0.5 - q2q2 - q3q3) + _4bz*(q1q3 - q0q2) - mx) + (-_4bx*q3+_4bz*q1)*(_4bx*(q1q2 - q0q3) + _4bz*(q0q1 + q2q3) - my) + _4bx*q2*(_4bx*(q0q2 + q1q3) + _4bz*(0.5 - q1q1 - q2q2) - mz);
    s1= _2q3*(2.0f*(q1q3 - q0q2) - ax) + _2q0*(2.0f*(q0q1 + q2q3) - ay) + -4.0f*q1*(2.0f*(0.5 - q1q1 - q2q2) - az) + _4bz*q3*(_4bx*(0.5 - q2q2 - q3q3) + _4bz*(q1q3 - q0q2) - mx) + (_4bx*q2+_4bz*q0)*(_4bx*(q1q2 - q0q3) + _4bz*(q0q1 + q2q3) - my) + (_4bx*q3-_8bz*q1)*(_4bx*(q0q2 + q1q3) + _4bz*(0.5 - q1q1 - q2q2) - mz);
    s2= -_2q0*(2.0f*(q1q3 - q0q2) - ax) + _2q3*(2.0f*(q0q1 + q2q3) - ay) + (-4.0f*q2)*(2.0f*(0.5 - q1q1 - q2q2) - az) + (-_8bx*q2-_4bz*q0)*(_4bx*(0.5 - q2q2 - q3q3) + _4bz*(q1q3 - q0q2) - mx)+(_4bx*q1+_4bz*q3)*(_4bx*(q1q2 - q0q3) + _4bz*(q0q1 + q2q3) -  my)+(_4bx*q0-_8bz*q2)*(_4bx*(q0q2 + q1q3) + _4bz*(0.5 - q1q1 - q2q2) - mz);
    s3= _2q1*(2.0f*(q1q3 - q0q2) - ax) + _2q2*(2.0f*(q0q1 + q2q3) - ay)+(-_8bx*q3+_4bz*q1)*(_4bx*(0.5 - q2q2 - q3q3) + _4bz*(q1q3 - q0q2) - mx)+(-_4bx*q0+_4bz*q2)*(_4bx*(q1q2 - q0q3) + _4bz*(q0q1 + q2q3) - my)+(_4bx*q1)*(_4bx*(q0q2 + q1q3) + _4bz*(0.5 - q1q1 - q2q2) - mz);
    norm = sqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
    s0 /= norm;
    s1 /= norm;
    s2 /= norm;
    s3 /= norm;

    // Apply feedback step
    qDot1 -= beta * s0;
    qDot2 -= beta * s1;
    qDot3 -= beta * s2;
    qDot4 -= beta * s3;
  }

  // Integrate rate of change of quaternion to yield quaternion
  q0 += qDot1 * dT;
  q1 += qDot2 * dT;
  q2 += qDot3 * dT;
  q3 += qDot4 * dT;

  // Normalise quaternion
  norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
  q0 /= norm;
  q1 /= norm;
  q2 /= norm;
  q3 /= norm;

  q.set(q1, q2, q3, q0);
}
