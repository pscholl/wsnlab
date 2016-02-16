/*
* Author: Sebastian Boettcher
* 
* MCU interface class, to be extended as needed
*
*/

#ifndef mcu_edison_h
#define mcu_edison_h

#include <string>
#include <vector>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include <math.h>
#include <time.h>
#include <unistd.h>


class mcu_edison {
 public:
  mcu_edison();
  ~mcu_edison();

  std::string readline();
  void write(std::string s);

 private:
  std::fstream m_tty;
  bool m_init;

};

#endif // mcu_edison_h

