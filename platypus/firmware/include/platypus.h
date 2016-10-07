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

#include "./mcu_edison.h"


class platypus {
 public:
  // standard constructor, if debug is set, data will be put to console
  // debug level:
  // 0 - no debug output
  // 1 - time and mem size every minute
  // 2 - full output every cycle
  // if sleep_enable is true, atom will try to sleep between mcu data transfers
  platypus(int debug, bool sleep_enabled, int alert_threshold, int rate);
  // destructor
  ~platypus();

  // init
  void mcu_init();

  // threading
  void spawn_threads();
  void join_threads();

  // enable or disable wifi, returns new status
  bool set_wifi_state(bool state);

  int get_bat() {return m_mcu_bat;}

 private:

  // mcu communication thread
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
  void writeHeader(uint32_t time, std::vector<int16_t> mag, std::vector<uint16_t> ldc, int32_t temp, uint32_t press, uint32_t hum, uint8_t bat);
  // write sensor data (IMU) to memory, given certain formats
  void writeData(std::vector<uint8_t> data);
  void writeData(std::vector<int16_t> data);
  void writeData(uint8_t data);
  void writeData(int16_t data);

  // save the current memory data buffer to the NAND-Flash
  // make sure to call this as async as it will lock until all data is written
  void writeDataToFlashIDX(uint8_t idx);
  void writeDataToFlash(std::vector<uint8_t> &data);

  // send the atom into sleep mode. processes are suspended, and resumed on wakeup
  void atomSleep();

  // print some sensor data etc. to console
  void printDebug(int &last_min, std::vector<float> data);


  //________________________________________________________________________________

  // pointer for the various device objects
  mcu_edison* m_mcu;

  // init flags
  bool m_mcu_init;

  // threading
  std::vector<std::thread> m_threads; // thread container
  std::atomic<bool> m_active; // flag denoting if threads should be running
  std::recursive_mutex m_mtx_time; // mutex for time() system call
  std::recursive_mutex m_mtx_write; // mutex for write to flash action

  std::atomic<bool> m_force_save; // deprecated, flag to force a save to flash
  std::atomic<bool> m_saving; // flag indicating save in progress (TODO: can we do this via mutex?)

  // the memory buffer, 2-dimensional to allow simultaneous read and write
  // index points to the vector that is currently to be used for writing
  std::array<std::vector<uint8_t>, 2> m_data_memory;
  uint8_t m_data_idx;

  // debug output level
  int m_debug;

  // flags for wifi/bt state
  bool m_wifi_enabled;
  bool m_bt_enabled;

  // gpio connected to the charger chip. active low (0 = charging)
  mraa::Gpio* m_charger_status;
  int m_alert_threshold;

  bool m_sleep_enabled;

  // ## Some mcu specific stuff ##

  // time at mcu initialization
  time_t m_mcu_init_stamp;

  // battery percentage from last mcu transmission
  uint8_t m_mcu_bat;

  // data sample rate (Hz)
  int m_mcu_rate;

};

#endif // platypus_h
