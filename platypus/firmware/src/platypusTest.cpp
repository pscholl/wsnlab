/*
* Author: Sebastian Boettcher
*
* Test file to check for platypus board errors, sensor malfunctions, etc...
*
*/

#include <boost/program_options.hpp>

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "./imu_edison.h"
#include "./display_edison.h"
#include "./ldc_edison.h"
#include "./batgauge_edison.h"

namespace po = boost::program_options;

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
int m_ldc_ID = 0x3D;
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
  // Allowed options:
  //   -h [ --help ]         display help message
  //   --no-color            disable color output
  //   --id-only             only test for ID correctness
  //   --config arg          parse config file at path
  //   --dsp arg             on/off Display test
  //   --imu arg             on/off IMU test
  //   --env arg             on/off EnvSens test
  //   --ldc arg             on/off LDC test
  //   --bat arg             on/off BatGauge test

  // Declare the supported options.
  po::options_description opts("Allowed options");
  opts.add_options()
    ("help,h", "display help message")
    ("no-color", "disable color output")
    ("id-only", "only test for ID correctness")
    ("config", po::value<std::string>(), "parse config file at path")
    ("dsp", po::value<bool>(), "on/off Display test")
    ("imu", po::value<bool>(), "on/off IMU test")
    ("env", po::value<bool>(), "on/off EnvSens test")
    ("ldc", po::value<bool>(), "on/off LDC test")
    ("bat", po::value<bool>(), "on/off BatGauge test")
    ;

  // create and populate options map
  po::variables_map opts_vm;
  po::store(po::parse_command_line(argc, argv, opts), opts_vm);
  po::notify(opts_vm);

  // print help message
  if (opts_vm.count("help")) {
    std::cout << opts << "\n";
    exit(0);
  }

  // read config file
  if (opts_vm.count("config"))
    parseConfig(opts_vm["config"].as<std::string>());
  else
    parseConfig("/etc/platypus/platypus.conf");

  // handle misc options
  if (opts_vm.count("no-color"))
    m_colors = false;
  if (opts_vm.count("id-only"))
    m_idonly = true;

  // handle device options
  if (opts_vm.count("dsp"))
    m_start_dsp = opts_vm["dsp"].as<bool>();
  if (opts_vm.count("imu"))
    m_start_imu = opts_vm["imu"].as<bool>();
  if (opts_vm.count("env"))
    m_start_env = opts_vm["env"].as<bool>();
  if (opts_vm.count("ldc"))
    m_start_ldc = opts_vm["ldc"].as<bool>();
  if (opts_vm.count("bat"))
    m_start_bat = opts_vm["bat"].as<bool>();
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
std::map<std::string, std::string> getIPs() {
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
  if (m_start_dsp) {
    m_dsp = new display_edison(m_dsp_resolution, m_dsp_hands);
    m_dsp->startThread();
  }

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
  bool imu_test = true;
  bool env_test = true;
  bool ldc_test = true;
  bool bat_test = true;

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
      imu_test = false;
    printf("MAG ");
    if (!IDtest(m_imu->getMagID(), m_mag_ID))
      imu_test = false;

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
      env_test = false;

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

    if (!IDtest(m_ldc->getID(), m_ldc_ID))
      ldc_test = false;

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
      bat_test = false;

    if (!m_idonly) {
      printf("Voltage: %fV\n", m_bat->getVCell());
      printf("State of Charge: %i%%\n", m_bat->getSoC());
      printf("Alert Threshold: %i%%\n", m_bat->getAlertThreshold());
    }
    printf("\n");
    fflush(stdout);
  }


  // write results on display
  if (m_start_dsp) {
    m_dsp->clear();

    if (!m_start_imu)
      m_dsp->print("IMU: SKIP", 5, 5);
    else if (imu_test)
      m_dsp->print("IMU: OK", 5, 5);
    else
      m_dsp->print("IMU: FAILURE", 5, 5);

    if (!m_start_env)
      m_dsp->print("ENV: SKIP", 5, 15);
    else if (env_test)
      m_dsp->print("ENV: OK", 5, 15);
    else
      m_dsp->print("ENV: FAILURE", 5, 15);

    if (!m_start_ldc)
      m_dsp->print("LDC: SKIP", 5, 25);
    else if (ldc_test)
      m_dsp->print("LDC: OK", 5, 25);
    else
      m_dsp->print("LDC: FAILURE", 5, 25);

    if (!m_start_bat)
      m_dsp->print("BAT: SKIP", 5, 35);
    else if (bat_test)
      m_dsp->print("BAT: OK", 5, 35);
    else
      m_dsp->print("BAT: FAILURE", 5, 35);

    if (m_start_bat && bat_test)
      m_dsp->print(m_bat->getSoC(), 5 + (8 * 6), 35);

    std::map<std::string, std::string> IPs = getIPs();
    if (IPs.find("wlan0") == IPs.end())
      IPs["wlan0"] = "N/A";
    m_dsp->print(IPs["wlan0"], 5, 45);

    usleep(1000000);

    m_dsp->flush();
  }

  if (imu_test && env_test && ldc_test && bat_test)
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
    m_dsp->stopThread();

  if (imu_test && env_test && ldc_test && bat_test)
    return 0;
  else
    return 1;
}



