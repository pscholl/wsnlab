/*
* Author: Sebastian Boettcher
*
* MCU interface class, to be extended as needed
*
*/

#include "./mcu_edison.h"



//_______________________________________________________________________________________________________
mcu_edison::mcu_edison(time_t mcu_start_time, int rate) : m_init(false),
  m_io(), m_serial(m_io, "/dev/ttymcu0"), m_rate(rate)
{
  if (!m_serial.is_open()) {
    printf("[MCU] Error opening TTY!\n");
    fflush(stdout);
  } else {
    m_init = true;

    while (readline(1) != "") {} // flush the serial line (read and discard until empty line)

    updateTime(mcu_start_time);

    write(getStartStr(m_rate)); // init condition for MCU

    std::string rec = readline(10);

    if (rec.find("[MCU] start") != std::string::npos) {
      m_mcu_init_stamp = std::stoi(rec.substr(12, std::string::npos));
      printf("[MCU] Running.\n");
      fflush(stdout);
    } else {
      printf("[MCU] Error initializing MCU!\n");
      fflush(stdout);
      m_init = false;
    }
  }
}


//_______________________________________________________________________________________________________
mcu_edison::~mcu_edison() {
  if (m_init) {
    write("mcu_stop"); // stop condition for MCU

    std::string rec = readline(10);

    if (rec.find("[MCU] stop") != std::string::npos) {
      printf("[MCU] Stopped.\n");
      fflush(stdout);
    } else {
      printf("[MCU] Error stopping MCU!\n");
      fflush(stdout);
    }
  }

  m_serial.close();
}



//_______________________________________________________________________________________________________
void mcu_edison::parseData(int &timestamp, size_t imu_length, std::vector<std::vector<int16_t>> &imu, std::vector<int16_t> &mag, std::vector<uint16_t> &ldc, int32_t &env_temp, uint32_t &env_press, uint32_t &env_hum, uint8_t &bat) {
  assert(ldc.size() == 2);
  assert(mag.size() == 3);

  if (!m_init)
    return;

  std::string line;
  while (line.find("data_end") == std::string::npos) {
    line = readline();

    if (line.size() <= 1)
      continue;

    // parse timecode and convert to platypus timestamp format
    if (line.find("Timecode1") != std::string::npos) {
      m_current_stamp = std::stoi(line.substr(line.find(':') + 2, std::string::npos));
      timestamp = (m_current_stamp - m_mcu_init_stamp) / 1000;

    // parse IMU temperature (make sure we dont read BME value here)
    } else if (line.find("Temperature") != std::string::npos && line.find("BME") == std::string::npos) {
      //temp = std::stoi(line.substr(line.find(':') + 2, std::string::npos));
      // Note: currently ignored since we have BME value

    // parse ADC values
    } else if (line.find("ADC 0") != std::string::npos) {
      ldc[0] = std::stoi(line.substr(line.find(':') + 2, std::string::npos));
    } else if (line.find("ADC 1") != std::string::npos) {
      ldc[1] = std::stoi(line.substr(line.find(':') + 2, std::string::npos));

    // parse IMU compass values
    } else if (line.find("Compass") != std::string::npos) {
      std::string unsplit = line.substr(line.find(':') + 2, std::string::npos);
      std::vector<std::string> compass;
      boost::split(compass, unsplit, boost::is_any_of(", "), boost::token_compress_on);
      assert(compass.size() == 3);
      mag[0] = std::stoi(compass[0]);
      mag[1] = std::stoi(compass[1]);
      mag[2] = std::stoi(compass[2]);

    // parse Bosch sensor values
    } else if (line.find("Temperature(BME)") != std::string::npos) {
      env_temp = std::stoi(line.substr(line.find(':') + 2, std::string::npos));
    } else if (line.find("Pressure(BME)") != std::string::npos) {
      env_press = std::stoi(line.substr(line.find(':') + 2, std::string::npos));
    } else if (line.find("Humidity(BME)") != std::string::npos) {
      env_hum = std::stoi(line.substr(line.find(':') + 2, std::string::npos));

    // parse battery gauge SoC value
    } else if (line.find("State of Charge") != std::string::npos) {
      bat = std::stoi(line.substr(line.find(':') + 2, std::string::npos));

    // parse accel and gyro values
    } else if (line.find("IMU") != std::string::npos) {
      std::vector<unsigned char> data = readbinary(imu_length, 20);
      for (size_t i = 0; i < data.size(); i+=12) {
        std::vector<int16_t> tmp;
        tmp.push_back(data[i+1] << 8 | data[i+0]);
        tmp.push_back(data[i+3] << 8 | data[i+2]);
        tmp.push_back(data[i+5] << 8 | data[i+4]);
        tmp.push_back(data[i+7] << 8 | data[i+6]);
        tmp.push_back(data[i+9] << 8 | data[i+8]);
        tmp.push_back(data[i+11] << 8 | data[i+10]);
        imu.push_back(tmp);
      }
    }
  }
  while (readline(1) != "") {} // flush the serial line (read and discard until empty line)
}


//_______________________________________________________________________________________________________
std::string mcu_edison::readline(size_t timeout) {
  if (!m_init)
    return "";

  // attach input stream to buffer
  std::istream is(&m_buf);
  // read a line from the buffer
  std::string line;
  std::getline(is, line);

  // if the stream has reached the end of the buffer, or the extracted line is empty, start a new serial read operation
  // otherwise skip this and just return the line
  if (is.eof() || line.size() == 0) {

    // result objects, one of which will be filled later by set_result()
    boost::optional<boost::system::error_code> timer_result;
    boost::optional<boost::system::error_code> read_result;

    // timer that handles the timeout after given number of seconds
    boost::asio::deadline_timer timer(m_serial.get_io_service());
    timer.expires_from_now(boost::posix_time::seconds(timeout));

    // start the timer and bind the result handler to set_result()
    // if timeout == 0 do not start timer, i.e. wait forever
    if (timeout > 0)
      timer.async_wait(boost::bind(set_result, &timer_result, _1));
    // start an async read until the first occurrance of a newline, bind the result handler to set_result()
    async_read_until(m_serial, m_buf, '\n', boost::bind(set_result, &read_result, _1));

    // prepare io_service for a run operation
    m_serial.get_io_service().reset();

    // start the io_service and let it execute at most one handler (i.e. read once and not more)
    while (m_serial.get_io_service().run_one()) {
      // if the read operation returned successfully, cancel the timer and continue
      // if the timer fires before a read operation completed successfully, cancel the read and continue
      if (read_result) {
        timer.cancel();
      } else if (timer_result) {
        m_serial.cancel();
      }
    }

    // refresh the input stream
    is.rdbuf(&m_buf);

    // add the next line to what was left from the last read operation
    std::string add;
    std::getline(is, add);
    line += add;
  }

  //std::cout << "line (" << line.size() << "): " << line << std::endl;

  return line;
}

//_______________________________________________________________________________________________________
void mcu_edison::write(std::string s) {
  if (!m_init)
    return;

  // write the string to the serial port, this can be synchronous
  boost::asio::write(m_serial, boost::asio::buffer(s.c_str(), 64));
}

//_______________________________________________________________________________________________________
void mcu_edison::updateTime(time_t t) {
  struct tm * ti = localtime(&t);

  std::stringstream ss;
  ss << "mcu_time:";
  ss << (ti->tm_hour < 10 ? "0" : "") << ti->tm_hour << ":";
  ss << (ti->tm_min < 10 ? "0" : "") << ti->tm_min << ":";
  ss << (ti->tm_sec < 10 ? "0" : "") << ti->tm_sec << std::endl;

  write(ss.str());
}

//_______________________________________________________________________________________________________
std::string mcu_edison::getStartStr(size_t rate) {
  std::stringstream ss;
  ss << "mcu_start mcu_rate:" << ((rate < 10000) ? rate : 9999) << ":" << std::endl;
  return ss.str();
}


//_______________________________________________________________________________________________________
std::vector<unsigned char> mcu_edison::readbinary(size_t size, size_t timeout) {
  std::vector<unsigned char> data(size);

  if (!m_init)
    return data;

  // copy the excess bytes that are still in the streambuf from the last readline() call
  size_t cpy = boost::asio::buffer_copy(boost::asio::buffer(data, size), m_buf.data());
  // clear the copied bytes from the streambuf, so it is now empty and ready for the next readline()
  m_buf.consume(cpy);

  //std::cout << "size: " << size << std::endl;
  //std::cout << "cpy: " << cpy << std::endl;

  // since the async_read overwrites any content in the given buffer,
  // we need to use another buffer and concatenate the two after the read
  std::vector<unsigned char> tmp(size - cpy);

  // result objects, one of which will be filled later by set_result()
  boost::optional<boost::system::error_code> timer_result;
  boost::optional<boost::system::error_code> read_result;

  // timer that handles the timeout after given number of seconds
  boost::asio::deadline_timer timer(m_serial.get_io_service());
  timer.expires_from_now(boost::posix_time::seconds(timeout));

  // start the timer and bind the result handler to set_result()
  // if timeout == 0 do not start timer, i.e. wait forever
  if (timeout > 0)
    timer.async_wait(boost::bind(set_result, &timer_result, _1));
  // start an async read until the given number of bytes has been read, bind the result handler to set_result()
  // the number of bytes to read is the full length of the data block minus the number of bytes already read in the last readline()
  async_read(m_serial, boost::asio::buffer(tmp, size - cpy), boost::bind(set_result, &read_result, _1));

  // prepare io_service for a run operation
  m_serial.get_io_service().reset();

  // start the io_service and let it execute at most one handler (i.e. read once and not more)
  while (m_serial.get_io_service().run_one()) {
    // if the read operation returned successfully, cancel the timer and continue
    // if the timer fires before a read operation completed successfully, cancel the read and continue
    if (read_result) {
      timer.cancel();
    } else if (timer_result) {
      m_serial.cancel();
    }
  }

  // concatenate the read bytes from the async_read to the bytes copied from the last readline()
  for (size_t i = 0; i < size - cpy; ++i)
    data[cpy + i] = tmp[i];

  //for (size_t i = 0; i < data.size(); ++i) {
  //  if (data[i-3] == 'T' && data[i-2] == 'e' && data[i-1] == 'm' && data[i-0] == 'p')
  //    std::cout << i - 4 << std::endl;
  //}
  //for (size_t i = 0; i < data.size(); ++i)
  //  std::cout << data[i];
  //std::cout << std::endl;


  return data;
}
