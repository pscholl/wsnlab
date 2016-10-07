/*
* Author: Sebastian Boettcher
*
* Main platypus runtime code. Reads command line and config file and
* uses the platypus class to spawn program threads.
*
*/

#include <boost/program_options.hpp>

#include <map>
#include <string>
#include <fstream>
#include <chrono>

#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "./platypus.h"

namespace po = boost::program_options;

int m_running = 1;

platypus* m_pps;

// default config
int m_log_level = 1;
int m_alert_threshold = 4;
bool m_start_mcu = true;
bool m_sleep_enabled = true;
int m_rate = 25;



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
  if (stat(path.c_str(), &buffer) != 0) { // TODO: this may be incorrect, throws exceptions? check if it works!
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

  // store configs TODO: throws if config not present! should ignore/warn
  m_log_level = std::stoi(cfg["log_level"]);
  m_alert_threshold = std::stoi(cfg["alert_threshold"]);
  m_start_mcu = stob(cfg["start_mcu"], m_start_mcu);
  m_sleep_enabled = stob(cfg["sleep_enabled"], m_sleep_enabled);
  m_rate = std::stoi(cfg["sample_rate"]);
}

//_______________________________________________________________________________________________________
void parseArgs(int argc, char** argv) {
  // Allowed options:
  //   -h [ --help ]         display help message
  //   --config arg          parse config file at path
  //   --mcu arg             on/off MCU operation
  //   --rate arg            data sample rate [Hz]
  //   --sleep arg           on/off sleep atom between data transfers
  //   --alert arg           value of battery alert threshold (shutdown if lower)
  //   --log arg             log output level

  // Declare the supported options.
  po::options_description opts("Allowed options");
  opts.add_options()
    ("help,h", "display help message")
    ("config", po::value<std::string>(), "parse config file at path")
    ("mcu", po::value<bool>(), "on/off MCU operation")
    ("rate", po::value<int>(), "data sample rate [Hz]")
    ("sleep", po::value<bool>(), "on/off sleep atom between data transfers")
    ("alert", po::value<int>(), "value of battery alert threshold (shutdown if reached)")
    ("log", po::value<int>(), "log output level")
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

  // handle device options
  if (opts_vm.count("mcu"))
    m_start_mcu = opts_vm["mcu"].as<bool>();
  if (opts_vm.count("rate"))
    m_rate = opts_vm["rate"].as<int>();
  if (opts_vm.count("sleep"))
    m_sleep_enabled = opts_vm["sleep"].as<bool>();
  if (opts_vm.count("alert"))
    m_alert_threshold = opts_vm["alert"].as<int>();
  if (opts_vm.count("log"))
    m_log_level = opts_vm["log"].as<int>();
}



//_______________________________________________________________________________________________________
int main(int argc, char** argv) {
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  parseArgs(argc, argv);

  std::cout << "[MAIN] config:" << std::endl;
  std::cout << "  start mcu:         " << m_start_mcu << std::endl;
  std::cout << "  sample rate:       " << m_rate << std::endl;
  std::cout << "  sleep enabled:     " << m_sleep_enabled << std::endl;
  std::cout << "  alert threshold:   " << m_alert_threshold << std::endl;
  std::cout << "  log level:         " << m_log_level << std::endl;

  m_pps = new platypus(m_log_level, m_sleep_enabled, m_alert_threshold, m_rate);

  // Start MCU
  if (m_start_mcu) {
    system("/etc/platypus/init_i2c8.sh > /dev/null 2>&1"); // mcu needs this to access i2c bus
    system("/etc/platypus/init_mcu_spi.sh > /dev/null 2>&1"); // mcu needs this to access spi bus
    m_pps->mcu_init();
  }

  int bat = 100;

  m_pps->spawn_threads();
  while (m_running == 1) {
    // check if the battery is low, exit nicely if yes
    bat = m_pps->get_bat();
    if (bat <= m_alert_threshold)
      break;
    usleep(100000);
  }
  m_pps->join_threads();

  delete m_pps;

  if (bat <= m_alert_threshold)
    system("shutdown now");

  return 0;
}
