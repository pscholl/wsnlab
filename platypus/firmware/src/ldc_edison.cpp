/*
* Author: Sebastian Boettcher
*
* Light-To-Digital Converter interface (TSL2584TSV)
*
*/

#include "./ldc_edison.h"



//_______________________________________________________________________________________________________
ldc_edison::ldc_edison(uint8_t i2c_addr) : m_ldc_address(i2c_addr), m_error(false) {
  m_i2c = new mraa::I2c(1); // LDC is on bus 1
  m_i2c->address(m_ldc_address);
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
  writeRegister(LDC_CONTROL, 0x00); // Power off device
  writeRegister(LDC_CONTROL, 0x01); // Power on device
  usleep(2000);
  writeRegister(LDC_TIMING, 0x6C); // Set integration timing to ca. 400ms
  writeRegister(LDC_INTERRUPT, 0x00); // Disable interrupts
  writeRegister(LDC_ANALOG, 0x02); // Analog gain 16x
  writeRegister(LDC_CONTROL, 0x03); // Enable ADC
}


//_______________________________________________________________________________________________________
std::vector<uint16_t> ldc_edison::getADC() {
  std::vector<uint16_t> data;
  if (m_error) {
    data.push_back(0);
    data.push_back(0);
    return data;
  }

  uint8_t ADC[4];
  m_i2c->address(m_ldc_address);

  m_i2c->writeByte((5<<5)+(LDC_DATA0LOW)); // prepare auto-increment transaction
  m_i2c->read(ADC, 4); // read 4 bytes
  data.push_back((ADC[1]<<8)+ADC[0]); // ADC0 value
  data.push_back((ADC[3]<<8)+ADC[2]); // ADC1 value
  return data;
}


//_______________________________________________________________________________________________________
std::string ldc_edison::getVersion() {
  std::stringstream ver;
  ver << std::to_string(m_ldc_address);
  ver << ":";
  ver << std::to_string(readRegister(LDC_ID)>>4);
  ver << "|";
  ver << std::to_string(readRegister(LDC_ID)&0xF);
  ver << "|";
  ver << std::to_string(readRegister(LDC_ID2)>>7);
  return ver.str();
}


//_______________________________________________________________________________________________________
uint8_t ldc_edison::readRegister(uint8_t addr, uint8_t i2c) {
  if (m_error)
    return 0;

  m_i2c->address(i2c);
  m_i2c->writeByte((1<<7)+(addr)); // prepare standard read transaction
  return m_i2c->readByte();
}
//_______________________________________________________________________________________________________
uint8_t ldc_edison::readRegister(uint8_t addr) {
  return readRegister(addr, m_ldc_address);
}


//_______________________________________________________________________________________________________
void ldc_edison::writeRegister(uint8_t addr, uint8_t data, uint8_t i2c) {
  m_i2c->address(i2c);
  m_i2c->writeByte((1<<7)+(addr)); // prepare standard read transaction
  int res = m_i2c->writeByte(data);
  if (res)
    m_error = true;
}
//_______________________________________________________________________________________________________
void ldc_edison::writeRegister(uint8_t addr, uint8_t data) {
  writeRegister(addr, data, m_ldc_address);
}


