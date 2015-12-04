/*
* Author: Sebastian Boettcher
* 
* Main Platypus class with threading, display interface, data collection, etc.
*
*/

#ifndef platypus_h
#define platypus_h

#include <string>
#include <vector>
#include <array>
#include <map>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include <chrono>

#include <math.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "mraa.hpp"

#include "./imu_edison.h"
#include "./display_edison.h"
#include "./mcu_edison.h"
#include "./batgauge_edison.h"
#include "./ldc_edison.h"


enum class DisplayStates {
  IDLE,
  INIT,
  OFF,
  CLOCK,
  MENU_BACK,
  MENU_WIFI,
  MENU_SAVE,
  MENU_STATS,
  STATS
};

#define MENU_TIME 5


class platypus {
 public:
  // standard constructor, if debug is set, data will be put to console
  // debug level:
  // 0 - no debug output
  // 1 - time and mem size every minute
  // 2 - full output every cycle
  platypus(int debug);
  // destructor
  ~platypus();

  // init
  void display_init(uint8_t clk_hands);
  imu_edison* imu_init(uint8_t i2c_addr, bool env_init);
  void mcu_init();
  void ldc_init();
  batgauge_edison* bat_init();

  // threading
  void spawn_threads();
  void join_threads();

  DisplayStates tap_event();

 private:

  // threads, display, imu, mcu communication
  void t_display();
  void t_imu();

  void t_mcu();

  // get current system (local) time as time structure
  struct tm * getTimeAndDate();
  
  // get time and date in 4 Byte as:
  // b4: YYYYYYMM
  // b3: MMDDDDDh
  // b2: hhhhmmmm
  // b1: mmssssss
  uint32_t get4ByteTimeAndDate();
  uint32_t timeToBytes(struct tm* t);
  struct tm bytesToTime(uint32_t b);

  // get ips of network interfaces in a map <interface name, ip string>
  std::map<std::string, std::string> getIPs();

  // write the header to memory with time, light, temperature, pressure, humidity
  void writeHeader();
  // write sensor data (IMU) to memory, given certain formats
  void writeData(std::vector<uint8_t> data);
  void writeData(std::vector<int16_t> data);
  void writeData(uint8_t data);
  void writeData(int16_t data);

  // save the current memory data buffer to the NAND-Flash
  // make sure to call this as async as it will lock until all data is written
  void writeDataToFlashIDX(uint8_t idx);
  void writeDataToFlash(std::vector<uint8_t> &data);

  // print some sensor data etc. to console
  void printDebug(int &last_min, std::vector<float> data);

  // print the menu strings with pointer at given entry
  void printMenu(int pos);

  //________________________________________________________________________________
  display_edison* m_dsp;
  imu_edison* m_imu;
  mcu_edison* m_mcu;
  ldc_edison* m_ldc;
  batgauge_edison* m_bat;

  bool m_dsp_init;
  bool m_imu_init;
  bool m_env_init;
  bool m_mcu_init;
  bool m_ldc_init;
  bool m_bat_init;

  std::vector<std::thread> m_threads;
  std::atomic<bool> m_active;
  std::recursive_mutex m_mtx_time;
  std::recursive_mutex m_mtx_write;

  bool m_force_save;

  std::vector<int16_t> m_imu_data;

  std::array<std::vector<uint8_t>, 2> m_data_memory;
  uint8_t m_data_idx;

  int m_debug;

  DisplayStates m_dsp_state;

  bool m_wifi_enabled;

};

#endif // platypus_h

