/*
* Author: Sebastian Boettcher
* 
* Test file to check for platypus board errors, sensor malfunctions, etc...
*
*/

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "./imu_edison.h"
#include "./display_edison.h"
#include "./ldc_edison.h"
#include "./batgauge_edison.h"

display_edison* m_dsp;
imu_edison* m_imu;
ldc_edison* m_ldc;
batgauge_edison* m_bat;


// default config
int m_i2c_bus = 1;
uint8_t m_mpu_address = 0x68;
uint8_t m_dsp_resolution = 3;
uint8_t m_dsp_hands = 3;
int m_log_level = 1;
int m_alert_threshold = 4;

bool m_start_imu = true;
bool m_start_ldc = true;
bool m_start_env = true;
bool m_start_dsp = true;
bool m_start_bat = true;

bool m_colors = true;
bool m_idonly = false;

int m_imu_ID = 0x71;
int m_mag_ID = 0x48;
int m_env_ID = 0x60;
std::string m_ldc_ID = "41:9|8|1";
int m_bat_ID = 3;



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
  m_start_bat = stob(cfg["start_bat"], m_start_bat);
  m_colors = stob(cfg["colors"], m_colors);
  m_idonly = stob(cfg["idonly"], m_idonly);
}


//_______________________________________________________________________________________________________
void parseArgs(int argc, char** argv) {
  // parse command line arguments
  // -h --help        | display help message
  // --no-color       | no color output
  // --id-only        | only test for ID correctness
  // --config:path    | parse config file at path
  // --dsp:[0/1]      | enable/disable display test
  // --imu:[0/1]      | enable/disable IMU test
  // --env:[0/1]      | enable/disable EnvSens test
  // --ldc:[0/1]      | enable/disable LDC test
  // --bat:[0/1]      | enable/disable BatGauge test
  //
  if (argc == 1) {
    parseConfig("./platypus.conf");
  } else {
    std::vector<std::string> args;
    args.assign(argv + 1, argv + argc);

    // check if custom config file is used, if not use ./platypus.conf
    bool cfg_chg = false;
    for (size_t i = 0; i < args.size(); ++i) {
      if (args[i].find("--config:") != std::string::npos)
        cfg_chg = true;
    }
    if (!cfg_chg)
      parseConfig("./platypus.conf");

    // parse arguments
    for (size_t i = 0; i < args.size(); ++i) {
      if (args[i] == "-h" || args[i] == "--help") {
        printf("Usage: %s <ARGS>\n", argv[0]);
        printf("\n");
        printf("No arguments: use config file './platypus.conf'\n");
        printf("\n");
        printf("arguments:\n");
        printf("\t -h --help\t| display help message\n");
        printf("\t --no-color\t| no color output\n");
        printf("\t --id-only\t| only test for ID correctness\n");
        printf("\t --config:path\t| parse config file at path\n");
        printf("\t --dsp:[0/1]\t| enable/disable display test\n");
        printf("\t --imu:[0/1]\t| enable/disable IMU test\n");
        printf("\t --env:[0/1]\t| enable/disable EnvSens test\n");
        printf("\t --ldc:[0/1]\t| enable/disable LDC test\n");
        printf("\t --bat:[0/1]\t| enable/disable BatGauge test\n");
        fflush(stdout);
        exit(0);
      }
      if (args[i] == "--no-color")
        m_colors = false;
      if (args[i] == "--id-only")
        m_idonly = true;
      if (args[i].find("--config:") != std::string::npos)
        parseConfig(args[i].substr(9));

      if (args[i].find("--dsp:") != std::string::npos)
        m_start_dsp = stob(args[i].substr(6), m_start_dsp);
      if (args[i].find("--imu:") != std::string::npos)
        m_start_imu = stob(args[i].substr(6), m_start_imu);
      if (args[i].find("--env:") != std::string::npos)
        m_start_env = stob(args[i].substr(6), m_start_env);
      if (args[i].find("--ldc:") != std::string::npos)
        m_start_ldc = stob(args[i].substr(6), m_start_ldc);
      if (args[i].find("--bat:") != std::string::npos)
        m_start_bat = stob(args[i].substr(6), m_start_bat);
    }
  }
}


//_______________________________________________________________________________________________________
void printR(std::string s) {
  if (m_colors)
    printf("\033[1;31m%s\033[0m", s.c_str());
  else
    printf("%s", s.c_str());
  fflush(stdout);
}
void printG(std::string s) {
  if (m_colors)
    printf("\033[1;32m%s\033[0m", s.c_str());
  else
    printf("%s", s.c_str());
  fflush(stdout);
}
void printY(std::string s) {
  if (m_colors)
    printf("\033[1;33m%s\033[0m", s.c_str());
  else
    printf("%s", s.c_str());
  fflush(stdout);
}

//_______________________________________________________________________________________________________
bool IDtest(int is, int target) {
  std::stringstream ss;
  ss << "ID: is '" << is << "', should be '" << target << "'\n";
  if (is == target)
    printG(ss.str());
  else
    printR(ss.str());
  return is == target;
}
bool IDtest(std::string is, std::string target) {
  std::stringstream ss;
  ss << "ID: is '" << is << "', should be '" << target << "'\n";
  if (is == target)
    printG(ss.str());
  else
    printR(ss.str());
  return is == target;
}


//_______________________________________________________________________________________________________
int main(int argc, char** argv) {
  parseArgs(argc, argv);

  printf("\n");
  printY("[TEST] Test conditions:\n");
  if (m_start_dsp)
    printG("\tDisplay\n");
  else
    printR("\tDisplay\n");

  if (m_start_imu)
    printG("\tIMU\n");
  else
    printR("\tIMU\n");

  if (m_start_imu && m_start_env)
    printG("\tEnvSens\n");
  else if (m_start_imu && !m_start_env)
    printR("\tEnvSens\n");
  else if (!m_start_imu && m_start_env)
    printR("\tskipping EnvSens (IMU needs to be enabled!)\n");
  else
    printR("\tEnvSens\n");

  if (m_start_ldc)
    printG("\tLDC\n");
  else
    printR("\tLDC\n");

  if (m_start_bat)
    printG("\tBatGauge\n");
  else
    printR("\tBatGauge\n");

  printf("\n");
  printY("[TEST] Init devices...\n");
  printf("\n");
  fflush(stdout);


  // Set up display
  if (m_start_dsp)
    m_dsp = new display_edison(m_dsp_resolution, m_dsp_hands);

  // Set up IMU
  if (m_start_imu) {
    m_imu = new imu_edison(m_i2c_bus, m_mpu_address, m_start_env);
    m_imu->setupIMU();
  }

  // Set up LDC
  if (m_start_ldc)
    m_ldc = new ldc_edison(m_i2c_bus);

  // Start battery gauge
  if (m_start_bat) {
    m_bat = new batgauge_edison(m_i2c_bus);
    m_bat->setAlertThreshold(m_alert_threshold);
  }

  printf("\n");

  usleep(1000000);


  // start testing
  bool success = true;

  // test display
  if (m_start_dsp) {
    printY("[TEST] Drawing clock on display...\n");
    printf("\n");

    m_dsp->clear();
    m_dsp->analogClock(true);
    m_dsp->flush();
  }

  // test IMU
  if (m_start_imu) {
    printY("[TEST] Testing IMU...\n");

    printf("IMU ");
    if (!IDtest(m_imu->getID(), m_imu_ID))
      success = false;
    printf("MAG ");
    if (!IDtest(m_imu->getMagID(), m_mag_ID))
      success = false;

    if (!m_idonly) {
      std::vector<float> data = m_imu->toReadable(m_imu->readRawIMU());
      float mx, my, mz;
      m_imu->getCompassData(mx, my, mz);

      printf("Temperature [DegC]:\n\t%f\n", data[6]);
      printf("Accelerometer [m/s^2]:\n");
      printf("\tX: %f\n", data[0]);
      printf("\tY: %f\n", data[1]);
      printf("\tZ: %f\n", data[2]);
      printf("Gyroscope [deg/s]:\n");
      printf("\tX: %f\n", data[3]);
      printf("\tY: %f\n", data[4]);
      printf("\tZ: %f\n", data[5]);
      printf("Compass [mGs]:\n");
      printf("\tX: %f\n", mx);
      printf("\tY: %f\n", my);
      printf("\tZ: %f\n", mz);
    }
    printf("\n");
    fflush(stdout);
  }

  // test EnvSens
  if (m_start_imu && m_start_env) {
    printY("[TEST] Testing environmental sensor...\n");

    if (!IDtest(m_imu->getEnvID(), m_env_ID))
      success = false;

    if (!m_idonly) {
      float T, P, H;
      m_imu->getEnvData(T, P, H);

      printf("Temperature [DegC]: %f\n", T);
      printf("Pressure [hPa]: %f\n", P);
      printf("Humidity [%%RH]: %f\n", H);
    }
    printf("\n");
    fflush(stdout);
  }

  // test LDC
  if (m_start_ldc) {
    printY("[TEST] Testing LDC...\n");

    if (!IDtest(m_ldc->getVersion(), m_ldc_ID))
      success = false;

    if (!m_idonly) {
      std::vector<uint16_t> LDC(2, 0);
      LDC = m_ldc->getADC();

      printf("ADC0 (vis/IR) [raw]: %i\n", LDC[0]);
      printf("ADC1 (IR) [raw]: %i\n", LDC[1]);
    }
    printf("\n");
    fflush(stdout);
  }

  // test BatGauge
  if (m_start_bat) {
    printY("[TEST] Testing battery gauge...\n");

    if (!IDtest(m_bat->getVersion(), m_bat_ID))
      success = false;

    if (!m_idonly) {
      printf("Voltage: %fV\n", m_bat->getVCell());
      printf("State of Charge: %i%%\n", m_bat->getSoC());
      printf("Alert Threshold: %i%%\n", m_bat->getAlertThreshold());
    }
    printf("\n");
    fflush(stdout);
  }

  if (success)
    printG("[TEST] All ID tests successful.\n");
  else
    printR("[TEST] Some ID tests failed!\n");


  if (m_start_bat)
    delete m_bat;
  if (m_start_ldc)
    delete m_ldc;
  if (m_start_imu)
    delete m_imu;
  if (m_start_dsp)
    delete m_dsp;

  if (success)
    return 0;
  else
    return 1;
}


