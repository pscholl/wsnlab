/*
* Author: Sebastian Boettcher
*
* Main Platypus class with threading, display interface, data collection, etc.
*
*/

#include "./platypus.h"

using namespace std::chrono;

//_______________________________________________________________________________________________________
platypus::platypus(int debug, bool sleep_enabled, int alert_threshold, int rate)
 :  m_mcu_init(false), m_active(false),
    m_force_save(false), m_saving(false), m_data_idx(0), m_debug(debug),
    m_wifi_enabled(true), m_bt_enabled(false),
    m_alert_threshold(alert_threshold), m_sleep_enabled(sleep_enabled),
    m_mcu_bat(100), m_mcu_rate(rate)
{
  m_charger_status = new mraa::Gpio(45);
  m_charger_status->dir(mraa::DIR_IN);
}


//_______________________________________________________________________________________________________
platypus::~platypus() {
  writeDataToFlashIDX(m_data_idx);
  if (m_mcu_init)
    delete m_mcu;
}


/*
 * Initializations
 */

//_______________________________________________________________________________________________________
void platypus::mcu_init() {
  m_mcu_init_stamp = mktime(getTimeAndDate());
  m_mcu = new mcu_edison(m_mcu_init_stamp, m_mcu_rate);
  m_mcu_init = m_mcu->isInit();
}



/*
 * Threading management
 */

//________________________________________________________________________________
void platypus::spawn_threads() {
  printf("[PLATYPUS] Spawning threads.\n");
  fflush(stdout);
  m_active = true;
  m_threads.push_back(std::thread(&platypus::t_mcu, this));
  pthread_setname_np(m_threads[0].native_handle(), "pps:t_mcu");
}

//________________________________________________________________________________
void platypus::join_threads() {
  printf("[PLATYPUS] Joining threads.\n");
  fflush(stdout);
  m_active = false;
  for (auto& th : m_threads) th.join();
  m_threads.clear();
}


/*
 * Functions called as threads
 */

//_______________________________________________________________________________________________________
void platypus::t_mcu() {
  std::vector<std::future<void>> handles; // collect all handles for async calls

  // initialize timestamps
  double sincelast = 0; // seconds since the last data transfer
  double rate = 0; // sample rate of the current sample batch (numsamples/sincelast)
  time_t timestamp_raw_1 = mktime(getTimeAndDate()); // timestamp at current data transfer
  time_t timestamp_raw_2 = m_mcu_init_stamp; // timestamp at last data transfer
  system_clock::time_point current_stamp = system_clock::now(); // the sample stamp, incremented by the period for each sample
  // TODO: init current_stamp as m_mcu_init_stamp?

  while (m_active) {
    if (!m_mcu_init)
      break;

    std::string line;
    // wait for the mcu to send data
    while (m_active && line.length() == 0)
      line = m_mcu->readline(1);

    // we want to escape before reading again if the main wants to join
    if (!m_active)
      break;

    // check for charger interrupt
    if (line.find("charging_start") != std::string::npos) {
      std::cout << "[MCU] charger connected" << std::endl;
      set_wifi_state(true);
      if (!m_saving) {
        // async call write function so data collection can continue while writing to flash
        handles.push_back(std::async(std::launch::async, &platypus::writeDataToFlashIDX, this, m_data_idx));
        // switch to other data collection vector
        m_data_idx = !m_data_idx;
      }
      continue;
    }
    // check for charger interrupt
    if (line.find("charging_stop") != std::string::npos) {
      std::cout << "[MCU] charger disconnected" << std::endl;
      if (!m_saving) {
        // async call write function so data collection can continue while writing to flash
        handles.push_back(std::async(std::launch::async, &platypus::writeDataToFlashIDX, this, m_data_idx));
        // switch to other data collection vector
        m_data_idx = !m_data_idx;
      }
      continue;
    }

    size_t imu_length;
    // check if receiving data
    if (line.find("data_start") != std::string::npos) {
      imu_length = std::stoi(line.substr(line.find(' ') + 1, std::string::npos));
      timestamp_raw_1 = mktime(getTimeAndDate());
      sincelast = difftime(timestamp_raw_1, timestamp_raw_2);
      timestamp_raw_2 = mktime(getTimeAndDate());
      //std::cout << "diff: " << sincelast << "s" << std::endl;
    } else {
      continue;
    }

    if (m_debug > 2) {
      printf("[MCU] Getting data...\n");
      fflush(stdout);
    }

    int stamp;
    std::vector<std::vector<int16_t>> imu;
    std::vector<int16_t> mag(3, 0);
    std::vector<uint16_t> ldc(2, 0);
    int32_t env_temp;
    uint32_t env_press;
    uint32_t env_hum;

    // recieve data from mcu
    m_mcu->parseData(stamp, imu_length, imu, mag, ldc, env_temp, env_press, env_hum, m_mcu_bat);
    //std::cout << "[MCU] Got " << imu.size() * 12 << " Bytes of IMU data." << std::endl;

    assert(imu_length == imu.size() * 12);

    rate = imu.size() / sincelast;

    if (abs(rate - m_mcu_rate) > 2)
      std::cout << "[WARN] discrepancy between config rate (" << m_mcu_rate << ") and actual rate (" << rate << ")!" << std::endl;

    //std::cout << "samples: " << imu.size() << std::endl;
    //std::cout << "rate: " << rate << std::endl;

    // write data to memory
    for (size_t i = 0; i < imu.size(); ++i) {
      // write Header after every 600 samples of data (32B header + 7200B data)
      if (m_data_memory[m_data_idx].size() % 7232 == 0) {
        std::time_t tmp = system_clock::to_time_t(current_stamp);
        struct tm * timestamp = localtime(&tmp);
        writeHeader(timeToBytes(timestamp), mag, ldc, env_temp, env_press, env_hum, m_mcu_bat);
      }

      writeData(imu[i]);
      // advance timestamp by the period of the current sample batch rate
      current_stamp += std::chrono::microseconds((int)(round(1000000.0/rate)));
    }

    m_mcu->updateTime(mktime(getTimeAndDate()));

    if (m_mcu_bat <= m_alert_threshold) {
      writeDataToFlashIDX(m_data_idx);
      printf("Battery: %d @ %s", m_mcu_bat, asctime(getTimeAndDate()));
      break;
    }

    // wait, otherwise mcu will wake up atom immediately
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // try to set the atom into sleep mode, until next mcu communication arrives
    // will not sleep if wifi cannot be disabled (currently charging)
    atomSleep();

    //usleep(100000);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  for (auto& h : handles) h.get(); // make sure all async calls return
}



/*
 * system info getter; direct system interaction
 */

//_______________________________________________________________________________________________________
struct tm * platypus::getTimeAndDate() {
  std::lock_guard<std::recursive_mutex> time_lock(m_mtx_time);

  time_t rawtime;
  time(&rawtime);
  return localtime(&rawtime);
}

//_______________________________________________________________________________________________________
uint32_t platypus::get4ByteTimeAndDate() {
  struct tm * tme = getTimeAndDate();
  return timeToBytes(tme);
}

//_______________________________________________________________________________________________________
uint32_t platypus::timeToBytes(struct tm * t) {
  // b4: YYYYYYMM
  uint8_t b4 = (((t->tm_year - 100) << 2) & 0xFC) | (((t->tm_mon + 1) & 0x0C) >> 2);
  // b3: MMDDDDDh
  uint8_t b3 = (((t->tm_mon + 1) & 0x03) << 6) | ((t->tm_mday & 0x1F) << 1) | ((t->tm_hour & 0x10) >> 4);
  // b2: hhhhmmmm
  uint8_t b2 = ((t->tm_hour & 0x0F) << 4) | ((t->tm_min & 0x3C) >> 2);
  // b1: mmssssss
  uint8_t b1 = ((t->tm_min & 0x03) << 6) | (t->tm_sec & 0x3F);

  // shift bytes in the right order
  uint32_t ret = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;

  return ret;
}

//_______________________________________________________________________________________________________
struct tm platypus::bytesToTime(uint32_t b) {
  struct tm tme;

  uint8_t b1 = b >> 24;
  uint8_t b2 = b >> 16;
  uint8_t b3 = b >> 8;
  uint8_t b4 = b;

  tme.tm_year = ((b4 & 0xFC) >> 2) + 100;
  tme.tm_mon = ((b4 & 0x03) << 2) + ((b3 & 0xC0) >> 6) - 1;
  tme.tm_mday = ((b3 & 0x3E) >> 1);
  tme.tm_hour = ((b3 & 0x01) << 4) + ((b2 & 0xF0) >> 4);
  tme.tm_min = ((b2 & 0x0F) << 2) + ((b1 & 0xC0) >> 6);
  tme.tm_sec = (b1 & 0x3F);

  return tme;
}

//_______________________________________________________________________________________________________
std::map<std::string, std::string> platypus::getIPs() {
  struct ifaddrs * ifAddrStruct = NULL;
  struct ifaddrs * ifa = NULL;
  void * tmpAddrPtr = NULL;
  std::map<std::string, std::string> IPs;

  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) {
      continue;
    }
    if (ifa->ifa_addr->sa_family == AF_INET) { // check if it is IPv4
      tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
      IPs[ifa->ifa_name] = addressBuffer;
    } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check if it is IPv6
      tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
      char addressBuffer[INET6_ADDRSTRLEN];
      inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
      IPs[ifa->ifa_name] = addressBuffer;
    }
  }

  if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);
  return IPs;
}

//_______________________________________________________________________________________________________
bool platypus::set_wifi_state(bool state) {
  if (state && !m_wifi_enabled) {
    system("rfkill unblock wifi");
    std::cout << "[WIFI] enabled" << std::endl;
    m_wifi_enabled = true;
  } else if (!state && m_wifi_enabled && m_charger_status->read() != 0) {
    // should only be able to block if charger not connected
    std::cout << "[WIFI] disabling" << std::endl;
    system("rfkill block wifi");
    m_wifi_enabled = false;
  }

  return m_wifi_enabled;
}

//_______________________________________________________________________________________________________
void platypus::atomSleep() {
  // make absolutely sure wifi is enabled and don't go into sleep if charger is connected
  if (m_charger_status->read() == 0) {
    system("rfkill unblock wifi");
    m_wifi_enabled = true;
  }
  // make sure wifi is disabled before sleep. if not, don't sleep
  else if (m_sleep_enabled && m_charger_status->read() != 0 && !set_wifi_state(false)) {
    std::cout << "[MAIN] going to sleep (" << m_mcu_bat << "%%, " << m_data_memory[m_data_idx].size() << "B)" << std::endl;
    std::ofstream powerstate;
    powerstate.open("/sys/power/state", std::ios::out | std::ios::trunc);
    powerstate << "mem";
    powerstate.close();
  }
}



/*
 * data write functions
 */

//_______________________________________________________________________________________________________
void platypus::writeHeader(uint32_t time, std::vector<int16_t> mag, std::vector<uint16_t> ldc, int32_t temp, uint32_t press, uint32_t hum, uint8_t bat) {
  assert(ldc.size() == 2);
  assert(mag.size() == 3);

  std::vector<uint8_t> header;

  // header consists of:
  // 4 Byte date and time
  header.push_back((time & 0xFF000000) >> 24);
  header.push_back((time & 0xFF0000) >> 16);
  header.push_back((time & 0xFF00) >> 8);
  header.push_back(time & 0xFF);
  // 6 Byte current magnetometer values
  header.push_back((mag[0] & 0xFF00) >> 8);
  header.push_back(mag[0] & 0xFF);
  header.push_back((mag[1] & 0xFF00) >> 8);
  header.push_back(mag[1] & 0xFF);
  header.push_back((mag[2] & 0xFF00) >> 8);
  header.push_back(mag[2] & 0xFF);
  // 2 Byte current visible/IR light value
  header.push_back((ldc[0] & 0xFF00) >> 8);
  header.push_back(ldc[0] & 0xFF);
  // 2 Byte current IR light value
  header.push_back((ldc[1] & 0xFF00) >> 8);
  header.push_back(ldc[1] & 0xFF);
  // 4 Byte current temperature value
  header.push_back((temp & 0xFF000000) >> 24);
  header.push_back((temp & 0xFF0000) >> 16);
  header.push_back((temp & 0xFF00) >> 8);
  header.push_back(temp & 0xFF);
  // 4 Byte current pressure value
  header.push_back((press & 0xFF000000) >> 24);
  header.push_back((press & 0xFF0000) >> 16);
  header.push_back((press & 0xFF00) >> 8);
  header.push_back(press & 0xFF);
  // 4 Byte current humidity value
  header.push_back((hum & 0xFF000000) >> 24);
  header.push_back((hum & 0xFF0000) >> 16);
  header.push_back((hum & 0xFF00) >> 8);
  header.push_back(hum & 0xFF);
  // 1 Byte current battery percentage
  header.push_back(bat);
  // 5 Byte empty space for possible later use
  header.push_back(0);
  header.push_back(0);
  header.push_back(0);
  header.push_back(0);
  header.push_back(0);

  // => 4 + 6 + 2 + 2 + 4 + 4 + 4 + 1 + 5 = 32 Byte

  for (size_t i = 0; i < header.size(); ++i)
    m_data_memory[m_data_idx].push_back(header[i]);

  if (m_debug > 2) {
    printf("[PLATYPUS] Header written at %d Bytes.\n", m_data_memory[m_data_idx].size()-8);
    fflush(stdout);
  }
}

//_______________________________________________________________________________________________________
void platypus::writeData(std::vector<uint8_t> data) {
  for (size_t i = 0; i < data.size(); ++i)
    writeData(data[i]);
}
//_______________________________________________________________________________________________________
void platypus::writeData(std::vector<int16_t> data) {
  for (size_t i = 0; i < data.size(); ++i)
    writeData(data[i]);
}
//_______________________________________________________________________________________________________
void platypus::writeData(uint8_t data) {
  m_data_memory[m_data_idx].push_back(data);
}
//_______________________________________________________________________________________________________
void platypus::writeData(int16_t data) {
  m_data_memory[m_data_idx].push_back((data & 0xFF00) >> 8);
  m_data_memory[m_data_idx].push_back(data & 0xFF);
}


//_______________________________________________________________________________________________________
void platypus::writeDataToFlashIDX(uint8_t idx) {
  writeDataToFlash(m_data_memory[idx]);
}


//_______________________________________________________________________________________________________
void platypus::writeDataToFlash(std::vector<uint8_t> &data) {
  if (data.size() == 0)
    return;

  std::lock_guard<std::recursive_mutex> write_lock(m_mtx_write);

  m_saving = true;

  std::stringstream filename;
  filename << "/home/root/pps_logs/";

  // search for already saved data files and choose new filename
  int filenum = 0;
  DIR *dir;
  struct dirent *ent;

  while ((dir = opendir(filename.str().c_str())) == NULL) {
    mkdir(filename.str().c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    printf("[PLATYPUS] Created directory %s\n", filename.str().c_str());
  }

  while ((ent = readdir(dir)) != NULL) {
    std::string curr(ent->d_name);
    if (curr.find("datalog") != std::string::npos) {  // found log file
      int begin = curr.find("datalog") + 7;  // get pos of first digit
      std::string sub = curr.substr(begin, 4);  // get substring of number
      if (std::stoi(sub) + 1 > filenum)
        filenum = std::stoi(sub) + 1;  // set new file number
    }
  }
  closedir(dir);


  // set new filename as /datalogXXXX.bin
  filename << "datalog" << std::setfill('0') << std::setw(4) << filenum << ".bin";

  printf("[PLATYPUS] Saving %d Bytes to file %s\n", data.size(), filename.str().c_str());
  fflush(stdout);

  // open file stream, put 8bit chars from memory to file
  std::fstream outfile;
  outfile.open(filename.str(), std::ofstream::out | std::ofstream::app | std::ofstream::binary);
  std::streambuf * sbuf = outfile.rdbuf(); // want to use sputc()
  for (size_t i = 0; i < data.size(); ++i) {
    sbuf->sputc(data[i]);
  }
  outfile.close();

  printf("[PLATYPUS] Writing done.\n");
  fflush(stdout);

  // clear data vector and shrink allocated memory down to 0 again
  data.clear();
  data.shrink_to_fit();

  m_saving = false;
}


/*
 * other functions
 */

// TODO: redo this so it works!
//_______________________________________________________________________________________________________
void platypus::printDebug(int &last_min, std::vector<float> data) {
  if (data.size() < 7)
    return;

  struct tm * t = getTimeAndDate();

  if (m_debug == 3) {
    printf("Time: %d-%d-%d %d:%d:%d\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

    printf("Temperature [C]:\n\t%f\n", data[6]);

    printf("Accelerometer [m/s^2]:\n");
    printf("\tX: %f\n", data[0]);
    printf("\tY: %f\n", data[1]);
    printf("\tZ: %f\n", data[2]);
    printf("Gyroscope [deg/s]:\n");
    printf("\tX: %f\n", data[3]);
    printf("\tY: %f\n", data[4]);
    printf("\tZ: %f\n", data[5]);

    if (m_data_memory[m_data_idx].size() > 1048576)
      printf("data size [MiB]:\n\t%.3f\n", m_data_memory[m_data_idx].size() / 1048576.0);
    else if (m_data_memory[m_data_idx].size() > 1024)
      printf("data size [KiB]:\n\t%.2f\n", m_data_memory[m_data_idx].size() / 1024.0);
    else
      printf("data size [B]:\n\t%d\n", m_data_memory[m_data_idx].size());

    printf("\n");
    fflush(stdout);
  } else if (m_debug == 1 && abs(t->tm_min - last_min) >= 5) {
    printf("[PLATYPUS] %d-%d-%d %2d:%2d | ", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min);
    if (m_data_memory[m_data_idx].size() > 1048576)
      printf("%.3f MiB\n", m_data_memory[m_data_idx].size() / 1048576.0);
    else if (m_data_memory[m_data_idx].size() > 1024)
      printf("%.2f KiB\n", m_data_memory[m_data_idx].size() / 1024.0);
    else
      printf("%d B\n", m_data_memory[m_data_idx].size());

    fflush(stdout);

    last_min = t->tm_min;
  }
}
