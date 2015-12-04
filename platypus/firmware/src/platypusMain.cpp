#include <map>
#include <string>
#include <fstream>
#include <chrono>

#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "./platypus.h"
#include "./imu_edison.h"
#include "./batgauge_edison.h"

int m_running = 1;

platypus* m_pps;
batgauge_edison* m_bat;
imu_edison* m_imu;

// default config
uint8_t m_mpu_address = 0x68;
uint8_t m_dsp_hands   = 3;
int m_log_level = 1;
int m_alert_threshold = 4;

bool m_start_imu = true;
bool m_start_ldc = true;
bool m_start_env = true;
bool m_start_dsp = true;
bool m_start_mcu = false;
bool m_start_bat = true;

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds milliseconds;

Clock::time_point last_int;


//_______________________________________________________________________________________________________
void sig_handler(int signo) {
  if (signo == SIGINT || signo == SIGTERM) {
    printf("\n[MAIN] Exiting nicely...\n");
    fflush(stdout);
    m_running = 0;
  }
}

//_______________________________________________________________________________________________________
// string to bool
bool stob(std::string s) {
  if (s == "true" || s == "1")
    return true;
  else if (s == "false" || s != "1")
    return false;
  return false;
}

//_______________________________________________________________________________________________________
void parseConfig(std::string path) {
  struct stat buffer;
  if (stat(path.c_str(), &buffer) != 0) {
    printf("[MAIN] Error opening config file path \"%s\"\n", path.c_str());
    printf("[MAIN] Using default config instead.\n");
    fflush(stdout);
    return;
  }

  std::map<std::string, std::string> cfg;

  std::fstream cfg_file;
  cfg_file.open(path, std::fstream::in);

  // read configs from file
  while (!cfg_file.eof()) {
    std::string line;
    std::getline(cfg_file, line);

    // comment lines
    if (line.front() == '#' || line.front() == '/')
      continue;

    std::string key = line.substr(0, line.find(':'));
    std::string value = line.substr(line.find(':') + 1, std::string::npos);
    cfg[key] = value;
  }

  cfg_file.close();

  // store configs
  m_mpu_address = (uint8_t) std::stoi(cfg["mpu_address"]);
  m_dsp_hands   = (uint8_t) std::stoi(cfg["dsp_hands"]);
  m_log_level = std::stoi(cfg["log_level"]);
  m_alert_threshold = std::stoi(cfg["alert_threshold"]);
  m_start_imu = stob(cfg["start_imu"]);
  m_start_ldc = stob(cfg["start_ldc"]);
  m_start_env = stob(cfg["start_env"]);
  m_start_dsp = stob(cfg["start_dsp"]);
  m_start_mcu = stob(cfg["start_mcu"]);
  m_start_bat = stob(cfg["start_bat"]);
}

//_______________________________________________________________________________________________________
void imu_isr(void*) {
  uint8_t is = m_imu->getIntStatus();

  // do not handle interrupt if last was less than 250ms ago
  Clock::time_point now = Clock::now();
  milliseconds ms = std::chrono::duration_cast<milliseconds>(now - last_int);
  if (ms.count() < 250)
    return;

  if (m_imu->hasFIFOInt(is)) {
    printf("[IMU] FIFO interrupt\n");
    m_imu->FIFOrst();
  }
  if (m_imu->hasWOMInt(is)) {
    printf("[IMU] WOM interrupt\n");
    m_pps->tap_event();
  }
  fflush(stdout);
  last_int = Clock::now();
}


//_______________________________________________________________________________________________________
int main(int argc, char** argv) {
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  last_int = Clock::now();

  if (argc == 1)
    parseConfig("config.cfg");
  else
    parseConfig(argv[1]);

  m_pps = new platypus(m_log_level);

  // Set up IMU, LDC
  if (m_start_imu) {
    m_imu = m_pps->imu_init(m_mpu_address, m_start_env);
    mraa::Gpio* imu_interrupt = new mraa::Gpio(36);
    imu_interrupt->dir(mraa::DIR_IN);
    imu_interrupt->isr(mraa::EDGE_FALLING, imu_isr, NULL);
    imu_interrupt->mode(mraa::MODE_HIZ);
  }
  if (m_start_ldc) {
    m_pps->ldc_init();
  }

  // Set up display
  if (m_start_dsp)
    m_pps->display_init(m_dsp_hands);

  // Start MCU
  if (m_start_mcu)
    m_pps->mcu_init();

  // Start battery gauge
  if (m_start_bat) {
    m_bat = m_pps->bat_init();
    m_bat->setAlertThreshold(m_alert_threshold);
  }

  m_pps->spawn_threads();
  while (m_running == 1) {
    // check if the battery is low, exit nicely if yes
    if (m_start_bat && m_bat->getAlertStatus())
      break;
    usleep(100000);
  }
  m_pps->join_threads();

  delete m_pps;

  return 0;
}


