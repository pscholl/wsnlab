/*
* Author: Sebastian Boettcher
*
* Light-To-Digital Converter interface (TSL25723FN)
*
*/

#include "./ldc_edison.h"



//_______________________________________________________________________________________________________
ldc_edison::ldc_edison(int i2c_bus, uint8_t i2c_addr) : m_ldc_address(i2c_addr) {
  m_i2c = new mraa::I2c(i2c_bus);
  m_i2c->address(m_ldc_address);
  init();
}


//_______________________________________________________________________________________________________
ldc_edison::~ldc_edison() {
}


//_______________________________________________________________________________________________________
void ldc_edison::setAddress(uint8_t i2c_addr) {
  m_ldc_address = i2c_addr;
}


//_______________________________________________________________________________________________________
void ldc_edison::init() {
  writeRegister(LDC_ENABLE, 0x00); // Power off device
  writeRegister(LDC_ENABLE, 0x01); // Power on device
  usleep(2000);
  writeRegister(LDC_ATIME, INTEGRATION_TIMING); // Integration timing
  writeRegister(LDC_CONTROL, ANALOG_GAIN); // Analog gain
  writeRegister(LDC_ENABLE, 0x03); // Enable ADC
}


//_______________________________________________________________________________________________________
std::vector<uint16_t> ldc_edison::getADC() {
  std::vector<uint16_t> data;

  uint8_t ADC[4];
  m_i2c->address(m_ldc_address);

  m_i2c->writeByte((5<<5)+(LDC_DATA0LOW)); // prepare auto-increment transaction

  try {
    m_i2c->read(ADC, 4); // read 4 bytes
  } catch (std::invalid_argument& e) {}

  data.push_back((ADC[1]<<8)+ADC[0]); // ADC0 value
  data.push_back((ADC[3]<<8)+ADC[2]); // ADC1 value
  return data;
}


//_______________________________________________________________________________________________________
uint8_t ldc_edison::getID() {
  return readRegister(LDC_ID);
}

//_______________________________________________________________________________________________________
float ldc_edison::getLux() {
  // get gain
  int gain;
  switch (ANALOG_GAIN) {
    case 0: gain = 1; break;
    case 1: gain = 8; break;
    case 2: gain = 16; break;
    case 3: gain = 120; break;
    default: gain = 120;
  }

  // get current value
  std::vector<uint16_t> data = getADC();

  // counts per lux  CPL = (ATIME_ms x AGAINx) / (GA x 60)
  float cpl = ((0xFF - INTEGRATION_TIMING + 1) * 2.73) * gain / 60;

  // fluorescent and incandescent light  Lux1 = (1 x C0DATA - 1.87 x C1DATA) / CPL
  float lux1 = (data[0] - 1.87 * data[1]) / cpl;

  // dimmed incandescent light  Lux2 = (0.63 x C0DATA - 1 x C1DATA) / CPL
  float lux2 = (0.63 * data[0] - data[1]) / cpl;

  // Lux = MAX(Lux1, Lux2, 0)
  if (0 > lux1 && 0 > lux2)
    return 0;
  else if (lux1 > lux2)
    return lux1;
  else
    return lux2;
}



//_______________________________________________________________________________________________________
uint8_t ldc_edison::readRegister(uint8_t addr, uint8_t i2c) {
  m_i2c->address(i2c);
  m_i2c->writeByte((1<<7)+(addr)); // prepare standard transaction

  uint8_t data = 0;

  try {
    data = m_i2c->readByte();
  } catch (std::invalid_argument& e) {}

  return data;
}
//_______________________________________________________________________________________________________
uint8_t ldc_edison::readRegister(uint8_t addr) {
  return readRegister(addr, m_ldc_address);
}


//_______________________________________________________________________________________________________
void ldc_edison::writeRegister(uint8_t addr, uint8_t data, uint8_t i2c) {
  m_i2c->address(i2c);
  m_i2c->writeByte((1<<7)+(addr)); // prepare standard transaction
  m_i2c->writeByte(data);
}
//_______________________________________________________________________________________________________
void ldc_edison::writeRegister(uint8_t addr, uint8_t data) {
  writeRegister(addr, data, m_ldc_address);
}
