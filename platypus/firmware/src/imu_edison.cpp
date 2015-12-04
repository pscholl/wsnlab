/*
* Author: Roshna George, Sebastian Boettcher
* Code for the Implementation of the IMU 9DOF on Edison Mini Breakout board
* Grove IMU based on MPU9150.
*
*/

#include "./imu_edison.h"



//_______________________________________________________________________________________________________
imu_edison::imu_edison(uint8_t i2c_addr, bool init_env)
 : m_mpu_address(i2c_addr), m_init_env(init_env)
{
  m_i2c = new mraa::I2c(0);
  m_i2c->address(m_mpu_address);
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
  return m_i2c->readReg(addr);
}

//_______________________________________________________________________________________________________
int16_t imu_edison::readRegister(uint8_t addrH, uint8_t addrL, uint8_t i2c) {
  m_i2c->address(i2c);

  uint8_t H = m_i2c->readReg(addrH);
  uint8_t L = m_i2c->readReg(addrL);

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

  // MPU init
  writeRegister(MPU_SMPLRT_DIV, 0x27, m_mpu_address); //set sample rate to 25Hz (rate=1kHz/(1+div))
  writeRegister(MPU_CONFIG, 0x06, m_mpu_address); //set DLPF_CFG to lowest bandwith (5 Hz @ Fs=1kHz)
  writeRegister(MPU_GYRO_CONFIG, 0x00, m_mpu_address); //Gyro Config, fs_sel to +-250 deg/s
  writeRegister(MPU_ACCEL_CONFIG, 0x08, m_mpu_address); //Accel Config, fs_sel to +-4g
  writeRegister(MPU_ACCEL_CONFIG_2, 0x00, m_mpu_address); //Accel Config 2, set A_DLPF_CFG to highest bandwith (460 Hz @ Fs=1kHz)
  writeRegister(MPU_FIFO_EN, 0x78, m_mpu_address); //enable fifo buffer for accel XYZ, gyro XYZ
  writeRegister(MPU_INT_PIN_CFG, 0xA0, m_mpu_address); //interrupt pin config
  writeRegister(MPU_INT_ENABLE, 0x50, m_mpu_address); //enable interrupt for FIFO and WoM
  writeRegister(MPU_MOT_THR, 0x80, m_mpu_address); //WoM threshold
  writeRegister(MPU_MOT_DETECT_CTRL, 0xC0, m_mpu_address); //WoM enable
  writeRegister(MPU_USER_CTRL, 0x64, m_mpu_address); //enable master i2c mode, enable FIFO, reset FIFO

  printf("[IMU] Setup done.\n");
  fflush(stdout);
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
  m_i2c->address(BME_I2C_ADDR);
  m_i2c->writeByte(BME_CALIB00);
  m_i2c->read(calib_1, 26);
  m_i2c->writeByte(BME_CALIB26);
  m_i2c->read(calib_2, 16);

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
std::vector<float> imu_edison::convertData_1tor(std::vector<int8_t> in) {
  return convertData_2tor(convertData_1to2(in));
}

//_______________________________________________________________________________________________________
std::vector<float> imu_edison::convertData_2tor(std::vector<int16_t> in) {
  std::vector<float> out;

  // convert accelerometer data according to MPU9150 data sheet
  out.push_back(in[0] / 8192.0 * 9.807);
  out.push_back(in[1] / 8192.0 * 9.807);
  out.push_back(in[2] / 8192.0 * 9.807);
  // convert gyroscope data according to MPU9150 data sheet
  out.push_back(in[3] / 131.0);
  out.push_back(in[4] / 131.0);
  out.push_back(in[5] / 131.0);
  // convert temperature sensor data according to MPU9150 data sheet
  out.push_back((in[6] / 340.0) + 35);

  return out;
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
  if (!m_init_env)
    return std::vector<uint8_t>();

  std::vector<uint8_t> data;
  uint8_t buffer[24];
  m_i2c->readBytesReg(MPU_EXT_SENS_DATA_00, buffer, 24);
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
  std::vector<uint8_t> env_data_raw = readESData();
  int32_t adc_T;
  int32_t adc_P;
  int32_t adc_H;

  adc_T  = (uint32_t)env_data_raw[5] >> 4;
  adc_T |= (uint32_t)env_data_raw[4] << 4;
  adc_T |= (uint32_t)env_data_raw[3] << 12;

  adc_P  = (uint32_t)env_data_raw[2] >> 4;
  adc_P |= (uint32_t)env_data_raw[1] << 4;
  adc_P |= (uint32_t)env_data_raw[0] << 12;

  adc_H = env_data_raw[7];
  adc_H |= (uint32_t)env_data_raw[6] << 8;

  comp_T = compT(adc_T);
  comp_P = compP(adc_P, adc_T);
  comp_H = compH(adc_H, adc_T);
}

