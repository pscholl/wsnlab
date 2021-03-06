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

//display_edison* m_dsp;  // display
platypus* m_pps;
batgauge_edison* m_bat;
imu_edison* m_imu;

// default config
int m_i2c_bus = 1;
uint8_t m_mpu_address = 0x68;
uint8_t m_dsp_resolution = 96;
uint8_t m_dsp_hands = 3;
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
    m_pps->exitNicely();
    m_pps->sendThis("quit");
    m_pps->m_active = 0;
    fflush(stdout);
    m_running = 0;
    exit(0);
  }
}

//_______________________________________________________________________________________________________
// string to bool
bool stob(std::string s, bool def = false) {
  if (s == "true" || s == "1")
    return true;
  else if (s == "")
    return def;
  else if (s == "false" || s == "0")
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
  } else {
    printf("[MAIN] Using config file \"%s\"\n", path.c_str());
    fflush(stdout);
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
  m_i2c_bus = (int) std::stoi(cfg["i2c_bus"]);
  m_mpu_address = (uint8_t) std::stoi(cfg["mpu_address"]);
  m_dsp_resolution = (uint8_t) std::stoi(cfg["dsp_resolution"]);
  m_dsp_hands = (uint8_t) std::stoi(cfg["dsp_hands"]);
  m_log_level = std::stoi(cfg["log_level"]);
  m_alert_threshold = std::stoi(cfg["alert_threshold"]);
  m_start_imu = stob(cfg["start_imu"], m_start_imu);
  m_start_ldc = stob(cfg["start_ldc"], m_start_ldc);
  m_start_env = stob(cfg["start_env"], m_start_env);
  m_start_dsp = stob(cfg["start_dsp"], m_start_dsp);
  m_start_mcu = stob(cfg["start_mcu"], m_start_mcu);
  m_start_bat = stob(cfg["start_bat"], m_start_bat);
}

//_______________________________________________________________________________________________________
void imu_isr(void*) {
  uint8_t is = m_imu->getIntStatus();
  Clock::time_point now = Clock::now();
  milliseconds ms = std::chrono::duration_cast<milliseconds>(now - last_int);
  if (ms.count() < 250)
    return;

  printf("[MAIN] imu_isr called.\n");
  m_pps->interrupt();

  if (m_imu->hasFIFOInt(is)) {
    printf("[MAIN] FIFO interrupt\n");
    m_imu->FIFOrst();
  }
  if (m_imu->hasWOMInt(is)) {
    DisplayStates tap = m_pps->tap_event();
    if (tap != DisplayStates::NOCHANGE)
      printf("[MAIN] WOM interrupt, new display state: %d\n", tap);
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
    parseConfig("./platypus.conf");
  else
    parseConfig(argv[1]);

  m_pps = new platypus(m_log_level);

  // Set up IMU, LDC
  if (m_start_imu) {
    m_imu = m_pps->imu_init(m_i2c_bus, m_mpu_address, m_start_env);
    mraa::Gpio* imu_interrupt = new mraa::Gpio(36);
    imu_interrupt->dir(mraa::DIR_IN);
    imu_interrupt->isr(mraa::EDGE_FALLING, imu_isr, NULL);
    imu_interrupt->mode(mraa::MODE_HIZ);
  }
  if (m_start_ldc) {
    m_pps->ldc_init(m_i2c_bus);
  }

  // Set up display
  if (m_start_dsp) {
    //m_pps->display_init(m_dsp_resolution, m_dsp_hands);
    /*
    m_dsp = new display_edison(m_dsp_resolution, m_dsp_hands);
    init_drawing();
    init_drawing();

    update_stickman();
    draw_stickman();
    printf("[TEST] Default Strichmann\n");
    std::cin.ignore();

    printf("[TEST] Befehl: Komme in den Bildschirm von rechts\n");
    join_display_from_right(2);
    std::cin.ignore();

    //    default_stickman();
    //    draw_stickman();
    //    printf("[TEST] Default Strichmann wieder hergestellt\n");
    //    std::cin.ignore();

    printf("[TEST] Befehl: Komme in den Bildschirm von links\n");
    join_display_from_left(2);
    std::cin.ignore();



    printf("[TEST] Strichmann springt %i Pixel nach rechts in %i er Schritten\n", s_r_distance, s_r_step);
    shift_right(s_r_distance, s_r_step);
    std::cin.ignore();
    printf("[TEST] Strichmann springt %i Pixel nach unten in %i er Schritten\n", s_d_distance, s_d_step);
    shift_down(s_d_distance,s_d_step);
    std::cin.ignore();
    printf("[TEST] Strichmann springt %i Pixel nach links in %i er Schritten\n", s_l_distance,s_l_step);
    shift_left(s_l_distance,s_l_step);
    std::cin.ignore();
    printf("[TEST] Strichmann springt %i Pixel nach oben in %i er Schritten\n", s_u_distance,s_u_step);
    shift_up(s_u_distance,s_u_step);
    std::cin.ignore();*/
  }

  // Start MCU
  if (m_start_mcu)
    m_pps->mcu_init();

  // Start battery gauge
  //if (m_start_bat) {
  //  m_bat = m_pps->bat_init(m_i2c_bus);
  //  m_bat->setAlertThreshold(m_alert_threshold);
  //}

  m_pps->spawn_threads();
  while (m_pps->m_active == 1) {
    // check if the battery is low, exit nicely if yes
    //if (m_start_bat && m_bat->getAlertStatus())
    //  break;
    usleep(100000);
  }

  m_pps->join_threads();

  delete m_pps;

  return 0;
}
