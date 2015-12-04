/*
* Author: Sebastian Boettcher
*
* MCU interface class, to be extended as needed
*
*/

#include "./mcu_edison.h"



//_______________________________________________________________________________________________________
mcu_edison::mcu_edison() : m_init(false) {
  m_tty.open("/dev/ttymcu0", std::fstream::in | std::fstream::out | std::fstream::app);
  if (!m_tty.is_open()) {
    printf("[MCU] Error opening TTY!\n");
    fflush(stdout);
  } else {
    m_init = true;
    write("mcu_start");

    std::string rec = readline();

    if (rec.find("[MCU] start") != std::string::npos) {
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
    write("mcu_stop");

    std::string rec = readline();

    //printf("%s", rec.c_str());
    //fflush(stdout);
    if (rec.find("[MCU] stop") != std::string::npos) {
      printf("[MCU] Stopped.\n");
      fflush(stdout);
    } else {
      printf("[MCU] Error stopping MCU!\n");
      fflush(stdout);
    }
  }

  m_tty.close();
}

//_______________________________________________________________________________________________________
std::string mcu_edison::readline() {
  if (!m_init)
    return "";

  m_tty.close();
  m_tty.open("/dev/ttymcu0", std::fstream::in | std::fstream::out | std::fstream::app);

  std::string rec;
  std::getline(m_tty, rec);
  rec += "\n";
  return rec;
}

//_______________________________________________________________________________________________________
void mcu_edison::write(std::string s) {
  if (!m_init)
    return;

  m_tty.close();
  m_tty.open("/dev/ttymcu0", std::fstream::in | std::fstream::out | std::fstream::app);

  m_tty.write(s.c_str(), s.size());
  m_tty.flush();
}
