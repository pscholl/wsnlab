/*
* Author: Sebastian Boettcher
*
* MCU interface class, to be extended as needed
*
*/

#ifndef mcu_edison_h
#define mcu_edison_h

#include <boost/algorithm/string.hpp>  // split

#include <boost/asio.hpp>  // serial_port, io_service, deadline_timer, streambuf
#include <boost/bind.hpp>  // bind set_result
#include <boost/optional.hpp>  // error_code
#include <boost/date_time/posix_time/posix_time.hpp>  // deadline_timer expiration

#include <string>
#include <vector>

#include <math.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>

class mcu_edison {
 public:
  mcu_edison(time_t mcu_start_time, int rate);
  ~mcu_edison();

  // parse the data sent into given variables
  // timestamp: seconds since mcu init
  void parseData(int &timestamp, size_t imu_length, std::vector<std::vector<int16_t>> &imu, std::vector<int16_t> &mag, std::vector<uint16_t> &ldc, int32_t &env_temp, uint32_t &env_press, uint32_t &env_hum, uint8_t &bat);

  // read line from serial until first newline (excluded)
  // times out after given number of seconds
  std::string readline(size_t timeout = 2);
  // write the string to serial
  void write(std::string s);

  // return initialize flag
  bool isInit() {return m_init;}

  // send time string to mcu
  void updateTime(time_t t);

  // constructs the start string with sample rate
  std::string getStartStr(size_t rate);

  // read given number of bytes from serial
  // times out after given number of seconds
  std::vector<unsigned char> readbinary(size_t size, size_t timeout = 2);

 private:

  // helper for read/wait call handling, fills the returned result b into the given result object a
  static void set_result (boost::optional<boost::system::error_code>* a, boost::system::error_code b) {
    a->reset(b);
  }

  bool m_init;

  // asio objects for read with timeout
  boost::asio::io_service m_io;
  boost::asio::serial_port m_serial;
  boost::asio::streambuf m_buf;

  // internal timestamp recieved from the mcu, in ms since mcu startup
  int m_mcu_init_stamp;
  int m_current_stamp;

  // sample rate
  int m_rate;

};

#endif // mcu_edison_h
