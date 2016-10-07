#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

#include <signal.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "./imu_edison.h"
#include "./display_edison.h"

int m_running = 1;

imu_edison* m_imu;
display_edison* m_dsp;

// default config
int m_i2c_bus = 6;
uint8_t m_mpu_address = 0x68;
uint8_t m_dsp_resolution = 128;
uint8_t m_dsp_hands = 3;


int minH = 4;
int maxH = 123;

int minX = 2;
int maxX = 41;
int minY = 44;
int maxY = 83;
int minZ = 86;
int maxZ = 125;

float scaleX = 2.0;
float scaleY = 2.0;
float scaleZ = 2.0;
int baseX = 22;
int baseY = 64;
int baseZ = 106;

size_t dataWidth = 120;
std::vector<std::vector<float>> data;

int rate = 10;


//_______________________________________________________________________________________________________
void sig_handler(int signo) {
  if (signo == SIGINT || signo == SIGTERM) {
    printf("\n[MAIN] Exiting nicely...\n");
    fflush(stdout);
    m_running = 0;
  }
}



//_______________________________________________________________________________________________________
void clearX() {
  Graphics_setForegroundColor(m_dsp->context(), ClrBlack);
  Graphics_setBackgroundColor(m_dsp->context(), ClrWhite);

  Graphics_Rectangle rect;
  rect.xMin = minH;
  rect.yMin = minX;
  rect.xMax = maxH;
  rect.yMax = maxX-1;
  Graphics_fillRectangle(m_dsp->context(), &rect);

  Graphics_setForegroundColor(m_dsp->context(), ClrWhite);
  Graphics_setBackgroundColor(m_dsp->context(), ClrBlack);
}
//_______________________________________________________________________________________________________
void clearY() {
  Graphics_setForegroundColor(m_dsp->context(), ClrBlack);
  Graphics_setBackgroundColor(m_dsp->context(), ClrWhite);

  Graphics_Rectangle rect;
  rect.xMin = minH;
  rect.yMin = minY;
  rect.xMax = maxH;
  rect.yMax = maxY-1;
  Graphics_fillRectangle(m_dsp->context(), &rect);

  Graphics_setForegroundColor(m_dsp->context(), ClrWhite);
  Graphics_setBackgroundColor(m_dsp->context(), ClrBlack);
}
//_______________________________________________________________________________________________________
void clearZ() {
  Graphics_setForegroundColor(m_dsp->context(), ClrBlack);
  Graphics_setBackgroundColor(m_dsp->context(), ClrWhite);

  Graphics_Rectangle rect;
  rect.xMin = minH;
  rect.yMin = minZ;
  rect.xMax = maxH;
  rect.yMax = maxZ-1;
  Graphics_fillRectangle(m_dsp->context(), &rect);

  Graphics_setForegroundColor(m_dsp->context(), ClrWhite);
  Graphics_setBackgroundColor(m_dsp->context(), ClrBlack);
}

//_______________________________________________________________________________________________________
void scaleDOWN(int i) {
  int newX = baseX - data[i][0] * scaleX;
  int newY = baseY - data[i][1] * scaleY;
  int newZ = baseZ - data[i][2] * scaleZ;

  if (newX < minX || newX > maxX)
    scaleX /= 2.0;
  if (newY < minY || newY > maxY)
    scaleY /= 2.0;
  if (newZ < minZ || newZ > maxZ)
    scaleZ /= 2.0;
}

//_______________________________________________________________________________________________________
void scaleUP() {
  bool upX = true;
  bool upY = true;
  bool upZ = true;
  int X, Y, Z;
  for (size_t i = 0; i < data.size(); ++i) {
    X = baseX - data[i][0] * scaleX * 2.0;
    Y = baseY - data[i][1] * scaleY * 2.0;
    Z = baseZ - data[i][2] * scaleZ * 2.0;
    if (X < minX || X > maxX)
      upX = false;
    if (Y < minY || Y > maxY)
      upY = false;
    if (Z < minZ || Z > maxZ)
      upZ = false;
  }

  if (scaleX < 2 && upX)
    scaleX *= 2.0;
  if (scaleY < 2 && upY)
    scaleY *= 2.0;
  if (scaleZ < 2 && upZ)
    scaleZ *= 2.0;
}

//_______________________________________________________________________________________________________
void redraw() {
  clearX();
  clearY();
  clearZ();

  scaleUP();

  int oldX, oldY, oldZ, newX, newY, newZ;
  for (size_t i = 1; i < data.size(); ++i) {
    scaleDOWN(i);
    oldX = baseX - data[i - 1][0] * scaleX;
    oldY = baseY - data[i - 1][1] * scaleY;
    oldZ = baseZ - data[i - 1][2] * scaleZ;
    newX = baseX - data[i][0] * scaleX;
    newY = baseY - data[i][1] * scaleY;
    newZ = baseZ - data[i][2] * scaleZ;
    Graphics_drawLine(m_dsp->context(), minH + i - 1, oldX, minH + i, newX);
    Graphics_drawLine(m_dsp->context(), minH + i - 1, oldY, minH + i, newY);
    Graphics_drawLine(m_dsp->context(), minH + i - 1, oldZ, minH + i, newZ);
  }

  //m_dsp->flush();
}

//_______________________________________________________________________________________________________
void getData() {
  std::vector<int16_t> fifo = m_imu->readFIFO();
  std::vector<float> tmp(3, 0);
  for (size_t i = 0; i < fifo.size(); i += 6) {
    tmp[0] = m_imu->accelToReadable(fifo[i]);
    tmp[1] = m_imu->accelToReadable(fifo[i + 1]);
    tmp[2] = m_imu->accelToReadable(fifo[i + 2]);
    if (data.size() < dataWidth) {
      data.push_back(tmp);
    } else {
      std::rotate(data.begin(), data.begin() + 1, data.end());
      data[dataWidth - 1] = tmp;
    }
  }
}


//_______________________________________________________________________________________________________
int main(int argc, char** argv) {
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  m_imu = new imu_edison(m_i2c_bus, m_mpu_address, false);
  m_imu->setupIMU();
  m_dsp = new display_edison(m_dsp_resolution, m_dsp_hands);

  m_dsp->clear();
  Graphics_drawLineH(m_dsp->context(), minH-1, maxH+1, maxX);
  Graphics_drawLineH(m_dsp->context(), minH-1, maxH+1, maxY);
  Graphics_drawLineH(m_dsp->context(), minH-1, maxH+1, maxZ);
  Graphics_drawLineV(m_dsp->context(), minH-1, minX, maxX); 
  Graphics_drawLineV(m_dsp->context(), minH-1, minY, maxY); 
  Graphics_drawLineV(m_dsp->context(), minH-1, minZ, maxZ); 
  m_dsp->flush();

  m_dsp->startThread();

  // make sure there is always some initial data
  data.push_back(std::vector<float>(3, 0));

  getData();

  printf("X: %f (%.2f)\nY: %f (%.2f)\nZ: %f (%.2f)", data[data.size()-1][0], scaleX, data[data.size()-1][1], scaleY, data[data.size()-1][2], scaleZ);

  std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
  std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

  int t = 0;
  while (m_running == 1) {
    t1 = std::chrono::high_resolution_clock::now();

    printf("\33[2K\33[1A");
    printf("\33[2K\33[1A");
    printf("\33[2K\33[1A");
    printf("\33[2K\r");
    printf("X: %f (%.2f)\nY: %f (%.2f)\nZ: %f (%.2f)", data[data.size()-1][0], scaleX, data[data.size()-1][1], scaleY, data[data.size()-1][2], scaleZ);
    fflush(stdout);

    getData();

    redraw();
    m_dsp->flush();
    ++t;

    t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    //std::cout << "\trate: " << 1.0/(duration/1000000.0) << "Hz" << std::endl;
    if (duration < 1000000.0 * (1.0 / rate))
      usleep(1000000.0 * (1.0 / rate) - duration);
    else
      std::cout << "rate (" << rate << "Hz) exceedance: " << 1.0/(duration/1000000.0) << "Hz; " << duration - (1000000.0 * (1.0 / rate)) << " us" << std::endl;
  }

  m_dsp->stopThread();

  return 0;
}


