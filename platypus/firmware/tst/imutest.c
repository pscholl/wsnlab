#include "mraa.h"

int main (int argc, const char* argv[]) {
  // init i2c comms
  mraa_i2c_context i2c = mraa_i2c_init(1);
  mraa_i2c_address(i2c, 0x68);
  uint8_t rx_tx_buf[2];

  // reset
  rx_tx_buf[0] = 0x6B; // power management 1
  rx_tx_buf[1] = 0x80; // set reset bit
  mraa_i2c_write(i2c, rx_tx_buf, 2);
  usleep(200000); // wait for reset

  // wakeup
  rx_tx_buf[0] = 0x6B; // power management 1
  rx_tx_buf[1] = 0x00; // unset sleep bit
  mraa_i2c_write(i2c, rx_tx_buf, 2);

  usleep(200000);

  // read raw accelerometer values
  uint8_t ax_rawH = mraa_i2c_read_byte_data(i2c, 0x3B);
  uint8_t ax_rawL = mraa_i2c_read_byte_data(i2c, 0x3C);
  uint8_t ay_rawH = mraa_i2c_read_byte_data(i2c, 0x3D);
  uint8_t ay_rawL = mraa_i2c_read_byte_data(i2c, 0x3E);
  uint8_t az_rawH = mraa_i2c_read_byte_data(i2c, 0x3F);
  uint8_t az_rawL = mraa_i2c_read_byte_data(i2c, 0x40);

  int16_t ax_raw = (int16_t) ((ax_rawH<<8) + ax_rawL);
  int16_t ay_raw = (int16_t) ((ay_rawH<<8) + ay_rawL);
  int16_t az_raw = (int16_t) ((az_rawH<<8) + az_rawL);

  // convert to g
  float ax = ax_raw / 16384.0;
  float ay = ay_raw / 16384.0;
  float az = az_raw / 16384.0;

  printf("X: %f\n", ax);
  printf("Y: %f\n", ay);
  printf("Z: %f\n", az);

  return 0;
}
