/*
* Author: Sebastian Boettcher
*
* Main Platypus class with threading, display interface, data collection, etc.
*
*/

#include "./platypus.h"

/* -------  sockets  ------- */
#include "./socketprx.h"
#define BUF 1024
/* ------- / sockets  ------- */

// Sockets
#define BUF 1024
socket_t sock;

// ------------------------  SOCKETS  ------------------------------

/* socketlayer.c - für Linux/UNIX */ #include "socketprx.h"
/* Die Funktion gibt aufgetretene Fehler aus und * beendet die Anwendung. */
void error_exit(char *error_message) {
  fprintf(stderr, "%s: %s\n", error_message, strerror(errno));
  exit(EXIT_FAILURE);
}

int create_socket( int af, int type, int protocol ) {
  socket_t sock;
  const int y = 1;

  /* Erzeuge das Socket. */
  sock = socket(af, type, protocol);

  if (sock < 0)
    error_exit("Fehler beim Anlegen eines Sockets");

  /* Mehr dazu siehe Anmerkung am Ende des Listings ... */
  setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
  return sock;
}

/* Erzeugt die Bindung an die Serveradresse,
 * (genauer gesagt an einen bestimmten Port). */
void bind_socket(socket_t *sock, struct in6_addr adress, unsigned short port) {
  struct sockaddr_in6 server;
  memset( &server, 0, sizeof (server));

  server.sin6_family = AF_INET6;
  server.sin6_addr = adress;
  server.sin6_port = htons(port);

  if (bind(*sock, (struct sockaddr*)&server,sizeof(server)) < 0)
    error_exit("Kann das Socket nicht \"binden\"");
}

/* Teile dem Socket mit, dass Verbindungswünsche *
 von Clients entgegengenommen werden. */
void listen_socket( socket_t *sock ) {
  if(listen(*sock, 5) == -1)
    error_exit("Fehler bei listen");
}

/* Bearbeite die Verbindungswünsche von Clients. *
 Der Aufruf von accept() blockiert so lange, *
 bis ein Client Verbindung aufnimmt. */
void accept_socket(socket_t *socket, socket_t *new_socket) {
  struct sockaddr_in6 client;
  unsigned int len;
  len = sizeof(client);
  *new_socket = accept(*socket,(struct sockaddr *)&client, &len);

  if (*new_socket == -1)
    error_exit("Fehler bei accept");
}

/* Baut die Verbindung zum Server auf. */
void connect_socket(socket_t *sock, char *serv_addr, unsigned short port) {
  struct sockaddr_in6 server;
  struct hostent *host_info;
  //  unsigned long addr;
  memset( &server, 0, sizeof (server));

  //Sockets Layer Call: gethostbyname2()
  host_info = gethostbyname2(serv_addr,AF_INET6);
  if (host_info == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }

  memset((char *) &server, 0, sizeof(server));
  server.sin6_flowinfo = 0;
  server.sin6_family = AF_INET6;
  memmove((char *) &server.sin6_addr.s6_addr, (char *) host_info->h_addr, host_info->h_length);
  server.sin6_port = htons(port);

  /* Baue die Verbindung zum Server auf. */
  if (connect(*sock, (struct sockaddr *) &server, sizeof( server)) < 0)
    error_exit( "Kann keine Verbindung zum Server herstellen");

}

/* Daten versenden via TCP */
void TCP_send( socket_t *sock, char *data, size_t size) {
  if(send( *sock, data, size, 0) == -1 )
    error_exit("Fehler bei send()");
}

/* Daten empfangen via TCP */
void TCP_recv( socket_t *sock, char *data, size_t size) {
  unsigned int len;
  len = recv (*sock, data, size, 0);

  if( len > 0 || len != -1 )
    data[len] = '\0';
  else
    error_exit("Fehler bei recv()");
}

/* Socket schließen */
void close_socket( socket_t *sock ){
  close(*sock);
}
/* Unter Linux/UNIX ist nichts zu tun ... */
void cleanup(void){
  printf("Aufraeumarbeiten erledigt ...\n");
  return;
}

// ------------------------ / SOCKETS ------------------------------


//_______________________________________________________________________________________________________
platypus::platypus(int debug)
 :  m_dsp(NULL), m_imu(NULL),
    m_dsp_init(false), m_imu_init(false), m_env_init(false), m_mcu_init(false), m_ldc_init(false), m_bat_init(false), m_active(false),
    m_force_save(false), m_saving(false), m_data_idx(0), m_debug(debug), m_dsp_state(DisplayStates::IDLE),
    m_wifi_enabled(true), m_bt_enabled(false)
{
  m_imu_data = std::vector<int16_t>(7, 0);
}


//_______________________________________________________________________________________________________
platypus::~platypus() {
  writeDataToFlashIDX(m_data_idx);
  if (m_imu_init)
    delete m_imu;
  if (m_dsp_init)
    delete m_dsp;
  if (m_mcu_init)
    delete m_mcu;
  if (m_ldc_init)
    delete m_ldc;
  if (m_bat_init)
    delete m_bat;
}


/*
 * Initializations
 */

//_______________________________________________________________________________________________________
void platypus::display_init(uint8_t res, uint8_t clk_hands) {
  m_dsp = new display_edison(res, clk_hands);
  m_dsp_init = true;
}

//_______________________________________________________________________________________________________
imu_edison* platypus::imu_init(int i2c_bus, uint8_t i2c_addr, bool env_init) {
  m_imu = new imu_edison(i2c_bus, i2c_addr, env_init);

  //m_imu->sleep(false);
  m_imu->setupIMU();

  m_imu_init = true;
  m_env_init = env_init;

  return m_imu;
}

//_______________________________________________________________________________________________________
void platypus::mcu_init() {
  m_mcu = new mcu_edison;
  m_mcu_init = true;
}

//_______________________________________________________________________________________________________
void platypus::ldc_init(int i2c_bus) {
  m_ldc = new ldc_edison(i2c_bus);
  m_ldc_init = true;
}

//_______________________________________________________________________________________________________
batgauge_edison* platypus::bat_init(int i2c_bus) {
  m_bat = new batgauge_edison(i2c_bus);
  m_bat_init = true;
  return m_bat;
}


/*
 * Threading management
 */

//________________________________________________________________________________
void platypus::spawn_threads() {
  printf("[PLATYPUS] Spawning threads.\n");
  fflush(stdout);
  m_active = true;
  m_threads.push_back(std::thread(&platypus::t_display, this));
  m_threads.push_back(std::thread(&platypus::t_imu, this));
  m_threads.push_back(std::thread(&platypus::t_mcu, this));

  // Sockets
  m_threads.push_back(std::thread(&platypus::listener, this));
  m_threads.push_back(std::thread(&platypus::sender, this));

  pthread_setname_np(m_threads[0].native_handle(), "pps:t_display");
  pthread_setname_np(m_threads[1].native_handle(), "pps:t_imu");
  pthread_setname_np(m_threads[2].native_handle(), "pps:t_mcu");

  // Sockets
  pthread_setname_np(m_threads[2].native_handle(), "pps:listener");
  pthread_setname_np(m_threads[2].native_handle(), "pps:sender");
}

void platypus::parseIncommingMessage(char *buffer) {
  char delimiter[] = ":";
  const char *ptr_usr = "[";
  const char *ptr_nc = "nc";
  const char *ptr_update = " update\n\n";
  const char *ptr_response = " response\n\n";
  char *ptr;

  // initialisieren und ersten Abschnitt erstellen
  ptr = strtok(buffer, delimiter);

  // New user ID assigned.
  if (strcmp(ptr,ptr_nc) == 0) {
    ptr = strtok(NULL, delimiter);
    if(ptr != NULL) {
      int clientID = atoi(ptr);
      printf("I am user %d\n", clientID);
    }
  } else if (ptr[0] == ptr_usr[0]) {
    // [40]:update,data
    // [40]:response,data
    // 0]
    // [ == "["
    // 0 == "["
    // action
    // data
    ptr = ptr + 1;
    int from = atoi(ptr);
    printf("from: %d\n", from);

    ptr = strtok(NULL, delimiter);

    printf("action: \"%s\"=\"%s\"\n", ptr, ptr_update);

    if(strcmp(ptr,ptr_update) == 0) {
      printf("I will update\n");
    } else if(strcmp(ptr,ptr_response) == 0) {
      printf("I will respond\n");
    } else {
      printf("Bullshit\n");
    }
  }

}

void platypus::listener() {
  sock = create_socket(AF_INET6, SOCK_STREAM, 0);
  atexit(cleanup);

  char *myIP = "a::100";
  connect_socket(&sock, myIP, 15000);


  char *buffer = (char *)malloc (BUF);

  do {
    buffer[0] = '\0';
    TCP_recv (&sock, buffer, BUF-1);

    if(strcmp (buffer, "quit\n") != 0)
      printf ("\nNachricht erhalten: %s\n", buffer);

      parseIncommingMessage(buffer);
  } while (strcmp (buffer, "quit\n") != 0 && m_active);

  close_socket (&sock);
  exit(0);
}

void platypus::sender() {
  char *buffer = (char *)malloc (BUF);

  do {
    fgets (buffer, BUF, stdin);
    TCP_send (&sock, buffer, strlen (buffer));
  } while (strcmp (buffer, "quit\n") != 0 && m_active);
  close_socket (&sock);
  exit(0);
}

void platypus::sendThis(char *buffer) {
  if (m_active) {
    char *message = (char *)malloc (BUF);
    sprintf(message, "br:%s\n", buffer);
    TCP_send (&sock, message, strlen (message));
  }
}

//________________________________________________________________________________
void platypus::join_threads() {
  printf("[PLATYPUS] Joining threads.\n");
  fflush(stdout);
  m_active = false;
  for (auto& th : m_threads) th.join();
  m_threads.clear();
}


/*
 * Functions called as threads
 */

//_______________________________________________________________________________________________________
void platypus::t_display() {
  int last_min = 0;
  DisplayStates prev_dsp = m_dsp_state;
  int sec_counter = 0;

  m_dsp_state = DisplayStates::INIT;

  bool state_changed = false;

  while (m_active) {
    if (!m_dsp_init)
      break;

    std::vector<float> data = m_imu->toReadable(m_imu_data);

    printDebug(last_min, data);

    state_changed = false;

    switch (m_dsp_state) {
      // INIT: display welcome message, switch to OFF
      case DisplayStates::INIT:
        if (prev_dsp != m_dsp_state) {
          sec_counter = 0;
          m_dsp->clear();
          int res = m_dsp->resolution();
          m_dsp->print("WELCOME TO", res/2, res/2-5, true);
          m_dsp->print("PLATYPUS", res/2, res/2+5, true);
          m_dsp->flush();
        } else if (sec_counter < 5) {
          ++sec_counter;
        } else {
          sec_counter = 0;
          m_dsp_state = DisplayStates::OFF;
          state_changed = true;
        }
        break;

      // OFF: stop display
      case DisplayStates::OFF:
        if (prev_dsp != m_dsp_state) {
          m_dsp->stop();
        }
        break;

      // CLOCK: display analog clock and battery charge, update once per minute, switch to OFF after some time
      case DisplayStates::CLOCK:
        if (prev_dsp != m_dsp_state) {
          if (!m_dsp->is_active())
            m_dsp->init();
          m_dsp->clear();
          m_dsp->analogClock(true);
          if (m_bat_init)
            m_dsp->batteryCharge(m_bat->getSoC());
          m_dsp->flush();
        } else if (sec_counter < 180) {
          m_dsp->analogClock();
          if (m_bat_init && m_dsp->is_refreshed()) {
            m_dsp->batteryCharge(m_bat->getSoC());
            m_dsp->flush();
          }
          ++sec_counter;
        } else {
          sec_counter = 0;
          m_dsp_state = DisplayStates::OFF;
          state_changed = true;
        }
        break;

      // MENU_BACK: display complete menu, switch to CLOCK
      case DisplayStates::MENU_BACK:
        if (prev_dsp != m_dsp_state) {
          sec_counter = 0;
          printMenu(1);
          m_dsp->flush();
        } else if (sec_counter < MENU_TIME) {
          printMenu(1);
          printTimer(sec_counter);
          m_dsp->flush();
          ++sec_counter;
        } else {
          sec_counter = 0;
          m_dsp_state = DisplayStates::CLOCK;
          state_changed = true;
        }
        break;

      // MENU_WIFI: move menu pointer, toggle wifi and switch to CLOCK
      case DisplayStates::MENU_WIFI:
        if (prev_dsp != m_dsp_state) {
          sec_counter = 0;
          printMenu(2);
          m_dsp->flush();
        } else if (sec_counter < MENU_TIME) {
          printMenu(2);
          printTimer(sec_counter);
          m_dsp->flush();
          ++sec_counter;
        } else {
          if (m_wifi_enabled) {
            system("rfkill block wifi");
            m_wifi_enabled = false;
          } else {
            system("rfkill unblock wifi");
            m_wifi_enabled = true;
          }
          sec_counter = 0;
          m_dsp_state = DisplayStates::MENU_BACK;
          state_changed = true;
        }
        break;

      // MENU_BT: move menu pointer, toggle bluetooth and switch to CLOCK
      case DisplayStates::MENU_BT:
        if (prev_dsp != m_dsp_state) {
          sec_counter = 0;
          printMenu(3);
          m_dsp->flush();
        } else if (sec_counter < MENU_TIME) {
          printMenu(3);
          printTimer(sec_counter);
          m_dsp->flush();
          ++sec_counter;
        } else {
          if (m_bt_enabled) {
            system("rfkill block bluetooth");
            m_bt_enabled = false;
          } else {
            system("rfkill unblock bluetooth");
            m_bt_enabled = true;
          }
          sec_counter = 0;
          m_dsp_state = DisplayStates::MENU_BACK;
          state_changed = true;
        }
        break;

      // MENU_SAVE: move menu pointer, save data and switch to CLOCK
      case DisplayStates::MENU_SAVE:
        if (prev_dsp != m_dsp_state) {
          sec_counter = 0;
          printMenu(4);
          m_dsp->flush();
        } else if (sec_counter < MENU_TIME) {
          printMenu(4);
          printTimer(sec_counter);
          m_dsp->flush();
          ++sec_counter;
        } else {
          m_force_save = true;
          sec_counter = 0;
          m_dsp_state = DisplayStates::MENU_BACK;
          state_changed = true;
        }
        break;

      // MENU_STATS: move menu pointer, switch to STATS
      case DisplayStates::MENU_STATS:
        if (prev_dsp != m_dsp_state) {
          sec_counter = 0;
          printMenu(5);
          m_dsp->flush();
        } else if (sec_counter < MENU_TIME) {
          printMenu(5);
          printTimer(sec_counter);
          m_dsp->flush();
          ++sec_counter;
        } else {
          sec_counter = 0;
          m_dsp_state = DisplayStates::STATS;
          state_changed = true;
        }
        break;

        // MENU_CONFIG: move menu pointer, switch to CONFIG
      case DisplayStates::MENU_CONFIG:
        if (prev_dsp != m_dsp_state) {
          sec_counter = 0;
          printMenu(6);
          m_dsp->flush();
        } else if (sec_counter < MENU_TIME) {
          printMenu(6);
          printTimer(sec_counter);
          m_dsp->flush();
          ++sec_counter;
        } else {
          sec_counter = 0;
          m_dsp_state = DisplayStates::CONFIG;
          state_changed = true;
        }
        break;

      // STATS: display stats
      case DisplayStates::STATS:
        {
          m_dsp->clear();
          m_dsp->print("Accel [m/s^2]:", 5, 5);
          m_dsp->print(data[0], 15, 15, 2);
          m_dsp->print(data[1], 15, 25, 2);
          m_dsp->print(data[2], 15, 35, 2);
          m_dsp->print("Gyro [deg/s]:", 5, 45);
          m_dsp->print(data[3], 15, 55, 2);
          m_dsp->print(data[4], 15, 65, 2);
          m_dsp->print(data[5], 15, 75, 2);
          m_dsp->print("Temp [degC]:", 5, 85);
          m_dsp->print(data[6], 15, 95, 2);
          m_dsp->flush();
          break;
        }

        // CONFIG: display config
      case DisplayStates::CONFIG:
        {
          m_dsp->clear();
          std::map<std::string, std::string> IPs = getIPs();
          if (IPs.find("wlan0") == IPs.end())
            IPs["wlan0"] = "N/A";
          m_dsp->print("IP:", 5, 5);
          m_dsp->print(IPs["wlan0"], 15, 15);
          m_dsp->print("RAM [Bytes]:", 5, 25);
          m_dsp->print((int)m_data_memory[m_data_idx].size(), 15, 35);
          m_dsp->flush();
          break;
        }

      case DisplayStates::IDLE:
        break;
      default:
        break;
    }

    if (!state_changed)
      prev_dsp = m_dsp_state;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

//_______________________________________________________________________________________________________
void platypus::t_imu() {
  std::vector<std::future<void>> handles; // collect all handles for async calls

  m_imu->FIFOrst();

  while (m_active) {
    if (!m_imu_init)
      break;

    //m_imu_data = m_imu->readRawIMU();

    // write data to flash after 1024*1024*128 = 134217728 B = 128 MiB
    if (!m_saving && (m_force_save || m_data_memory[m_data_idx].size() >= 134217728)) {
      // async call write function so data collection can continue while writing to flash
      // TODO: get this to work with overloaded function writeDataToFlash(uint8_t)
      handles.push_back(std::async(std::launch::async, &platypus::writeDataToFlashIDX, this, m_data_idx));
      // switch to other data collection vector
      m_data_idx = !m_data_idx;
      m_force_save = false;
    }

    // only save accel and gyro data
    //std::vector<int16_t>::const_iterator first = m_imu_data.begin();
    //std::vector<int16_t>::const_iterator last = m_imu_data.end() - 1;
    //writeData(std::vector<int16_t>(first, last));

    // read values from FIFO and save them
    std::vector<int16_t> fifo_data = m_imu->readFIFO();
    writeData(fifo_data);
    if (fifo_data.size() >= 6) {
      for (size_t i = 0; i < 6; ++i)
        m_imu_data[i] = fifo_data[fifo_data.size() - 6 + i];
    }

    m_imu_data[6] = m_imu->readRawTemp();

    //usleep(100000);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  for (auto& h : handles) h.get(); // make sure all async calls return
}

//_______________________________________________________________________________________________________
void platypus::t_mcu() {
  while (m_active) {
    if (!m_mcu_init)
      break;

    //printf("%s", m_mcu->readline().c_str());
    //fflush(stdout);

    //usleep(100000);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}



/*
 * system info getter
 */

//_______________________________________________________________________________________________________
struct tm * platypus::getTimeAndDate() {
  std::lock_guard<std::recursive_mutex> time_lock(m_mtx_time);

  time_t rawtime;
  time(&rawtime);
  return localtime(&rawtime);
}

//_______________________________________________________________________________________________________
uint32_t platypus::get4ByteTimeAndDate() {
  struct tm * tme = getTimeAndDate();
  return timeToBytes(tme);
}

//_______________________________________________________________________________________________________
uint32_t platypus::timeToBytes(struct tm * t) {
  // b4: YYYYYYMM
  uint8_t b4 = (((t->tm_year - 100) << 2) & 0xFC) | (((t->tm_mon + 1) & 0x0C) >> 2);
  // b3: MMDDDDDh
  uint8_t b3 = (((t->tm_mon + 1) & 0x03) << 6) | ((t->tm_mday & 0x1F) << 1) | ((t->tm_hour & 0x10) >> 4);
  // b2: hhhhmmmm
  uint8_t b2 = ((t->tm_hour & 0x0F) << 4) | ((t->tm_min & 0x3C) >> 2);
  // b1: mmssssss
  uint8_t b1 = ((t->tm_min & 0x03) << 6) | (t->tm_sec & 0x3F);

  // shift bytes in the right order
  uint32_t ret = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;

  return ret;
}

//_______________________________________________________________________________________________________
struct tm platypus::bytesToTime(uint32_t b) {
  struct tm tme;

  uint8_t b1 = b >> 24;
  uint8_t b2 = b >> 16;
  uint8_t b3 = b >> 8;
  uint8_t b4 = b;

  tme.tm_year = ((b4 & 0xFC) >> 2) + 100;
  tme.tm_mon = ((b4 & 0x03) << 2) + ((b3 & 0xC0) >> 6) - 1;
  tme.tm_mday = ((b3 & 0x3E) >> 1);
  tme.tm_hour = ((b3 & 0x01) << 4) + ((b2 & 0xF0) >> 4);
  tme.tm_min = ((b2 & 0x0F) << 2) + ((b1 & 0xC0) >> 6);
  tme.tm_sec = (b1 & 0x3F);

  return tme;
}

//_______________________________________________________________________________________________________
std::map<std::string, std::string> platypus::getIPs() {
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


/*
 * data write functions
 */

//_______________________________________________________________________________________________________
void platypus::writeHeader() {
  if (!m_imu_init && !m_ldc_init && !m_env_init)
    return;

  uint32_t header_time = get4ByteTimeAndDate();
  std::vector<uint16_t> LDC(2, 0);
  int32_t temp = 0;
  uint32_t press = 0;
  uint32_t hum = 0;

  if (m_ldc_init)
    LDC = m_ldc->getADC();

  if (m_imu_init && m_env_init)
    m_imu->getEnvData(temp, press, hum);

  std::vector<uint8_t> header;

  // header consists of:
  // 4 Byte date and time
  header.push_back((header_time & 0xFF000000) >> 24);
  header.push_back((header_time & 0xFF0000) >> 16);
  header.push_back((header_time & 0xFF00) >> 8);
  header.push_back(header_time & 0xFF);
  // 2 Byte current visible/IR light value
  header.push_back((LDC[0] & 0xFF00) >> 8);
  header.push_back(LDC[0] & 0xFF);
  // 2 Byte current IR light value
  header.push_back((LDC[1] & 0xFF00) >> 8);
  header.push_back(LDC[1] & 0xFF);
  // 4 Byte current temperature value
  header.push_back((temp & 0xFF000000) >> 24);
  header.push_back((temp & 0xFF0000) >> 16);
  header.push_back((temp & 0xFF00) >> 8);
  header.push_back(temp & 0xFF);
  // 4 Byte current pressure value
  header.push_back((press & 0xFF000000) >> 24);
  header.push_back((press & 0xFF0000) >> 16);
  header.push_back((press & 0xFF00) >> 8);
  header.push_back(press & 0xFF);
  // 4 Byte current humidity value
  header.push_back((hum & 0xFF000000) >> 24);
  header.push_back((hum & 0xFF0000) >> 16);
  header.push_back((hum & 0xFF00) >> 8);
  header.push_back(hum & 0xFF);

  for (size_t i = 0; i < header.size(); ++i)
    m_data_memory[m_data_idx].push_back(header[i]);

  if (m_debug > 2) {
    printf("[PLATYPUS] Header written at %d Bytes.\n", m_data_memory[m_data_idx].size()-8);
    fflush(stdout);
  }
}

//_______________________________________________________________________________________________________
void platypus::writeData(std::vector<uint8_t> data) {
  if (!m_imu_init)
    return;

  for (size_t i = 0; i < data.size(); ++i)
    writeData(data[i]);
}
//_______________________________________________________________________________________________________
void platypus::writeData(std::vector<int16_t> data) {
  if (!m_imu_init)
    return;

  for (size_t i = 0; i < data.size(); ++i)
    writeData(data[i]);
}
//_______________________________________________________________________________________________________
void platypus::writeData(uint8_t data) {
  if (!m_imu_init)
    return;

  // write Header in the beginning and after every 600 samples of data (20B header + 7200B data)
  if (m_data_memory[m_data_idx].size() % 7220 == 0)
    writeHeader();

  m_data_memory[m_data_idx].push_back(data);
}
//_______________________________________________________________________________________________________
void platypus::writeData(int16_t data) {
  if (!m_imu_init)
    return;

  // write Header in the beginning and after every 600 samples of data (20B header + 7200B data)
  if (m_data_memory[m_data_idx].size() % 7220 == 0)
    writeHeader();

  m_data_memory[m_data_idx].push_back((data & 0xFF00) >> 8);
  m_data_memory[m_data_idx].push_back(data & 0xFF);
}


//_______________________________________________________________________________________________________
void platypus::writeDataToFlashIDX(uint8_t idx) {
  writeDataToFlash(m_data_memory[idx]);
}


//_______________________________________________________________________________________________________
void platypus::writeDataToFlash(std::vector<uint8_t> &data) {
  if (!m_imu_init)
    return;

  std::lock_guard<std::recursive_mutex> write_lock(m_mtx_write);

  m_saving = true;

  std::stringstream filename;
  filename << "/home/root/pps_logs/";

  // search for already saved data files and choose new filename
  int filenum = 0;
  DIR *dir;
  struct dirent *ent;

  while ((dir = opendir(filename.str().c_str())) == NULL) {
    mkdir(filename.str().c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    printf("[PLATYPUS] Created directory %s\n", filename.str().c_str());
  }

  while ((ent = readdir(dir)) != NULL) {
    std::string curr(ent->d_name);
    if (curr.find("datalog") != std::string::npos) {  // found log file
      int begin = curr.find("datalog") + 7;  // get pos of first digit
      std::string sub = curr.substr(begin, 4);  // get substring of number
      if (std::stoi(sub) + 1 > filenum)
        filenum = std::stoi(sub) + 1;  // set new file number
    }
  }
  closedir(dir);


  // set new filename as /datalogXXXX.bin
  filename << "datalog" << std::setfill('0') << std::setw(4) << filenum << ".bin";

  printf("[PLATYPUS] Saving %d Bytes to file %s\n", data.size(), filename.str().c_str());

  // open file stream, put 8bit chars from memory to file
  std::fstream outfile;
  outfile.open(filename.str(), std::ofstream::out | std::ofstream::app | std::ofstream::binary);
  std::streambuf * sbuf = outfile.rdbuf(); // want to use sputc()
  for (size_t i = 0; i < data.size(); ++i) {
    sbuf->sputc(data[i]);
  }
  outfile.close();

  printf("[PLATYPUS] Writing done.\n");
  fflush(stdout);

  // clear data vector and shrink allocated memory down to 0 again
  data.clear();
  data.shrink_to_fit();

  m_saving = false;
}


/*
 * other functions
 */

//_______________________________________________________________________________________________________
DisplayStates platypus::tap_event() {
  std::vector<float> imu_data = m_imu->toReadable(m_imu_data);
  std::vector<float> imu_curr = m_imu->toReadable(m_imu->readRawIMU());
  //printf("%f, %f, %f | %f, %f, %f\n", imu_data[0], imu_data[1], imu_data[2], imu_curr[3], imu_curr[4], imu_curr[5]);
  //fflush(stdout);

  // try to filter taps more or less accurately

  // platypus board needs to be level (parallel to ground)
  if (imu_data[0] > 1 || imu_data[0] < -1)
    return DisplayStates::NOCHANGE;
  if (imu_data[1] > 1 || imu_data[1] < -1)
    return DisplayStates::NOCHANGE;
  if (imu_data[2] < 8)
    return DisplayStates::NOCHANGE;

  // current gyro values need to be small
  if (imu_curr[3] > 45 || imu_curr[3] < -45)
    return DisplayStates::NOCHANGE;
  if (imu_curr[4] > 45 || imu_curr[4] < -45)
    return DisplayStates::NOCHANGE;
  if (imu_curr[5] > 45 || imu_curr[5] < -45)
    return DisplayStates::NOCHANGE;

  switch (m_dsp_state) {
    case DisplayStates::INIT:
      break;

    case DisplayStates::OFF:
      m_dsp_state = DisplayStates::CLOCK;
      break;

    case DisplayStates::CLOCK:
      m_dsp_state = DisplayStates::MENU_BACK;
      break;

    case DisplayStates::MENU_BACK:
      m_dsp_state = DisplayStates::MENU_WIFI;
      break;

    case DisplayStates::MENU_WIFI:
      m_dsp_state = DisplayStates::MENU_BT;
      break;

    case DisplayStates::MENU_BT:
      m_dsp_state = DisplayStates::MENU_SAVE;
      break;

    case DisplayStates::MENU_SAVE:
      m_dsp_state = DisplayStates::MENU_STATS;
      break;

    case DisplayStates::MENU_STATS:
      m_dsp_state = DisplayStates::MENU_CONFIG;
      break;

    case DisplayStates::MENU_CONFIG:
      m_dsp_state = DisplayStates::MENU_BACK;
      break;

    case DisplayStates::STATS:
      m_dsp_state = DisplayStates::MENU_BACK;
      break;

    case DisplayStates::CONFIG:
      m_dsp_state = DisplayStates::MENU_BACK;
      break;

    case DisplayStates::IDLE:
      break;
    default:
      break;
  }

  return m_dsp_state;
}

//_______________________________________________________________________________________________________
void platypus::printDebug(int &last_min, std::vector<float> data) {
  if (data.size() < 7)
    return;

  struct tm * t = getTimeAndDate();

  if (m_debug == 3) {
    printf("Time: %d-%d-%d %d:%d:%d\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

    printf("Temperature [C]:\n\t%f\n", data[6]);

    printf("Accelerometer [m/s^2]:\n");
    printf("\tX: %f\n", data[0]);
    printf("\tY: %f\n", data[1]);
    printf("\tZ: %f\n", data[2]);
    printf("Gyroscope [deg/s]:\n");
    printf("\tX: %f\n", data[3]);
    printf("\tY: %f\n", data[4]);
    printf("\tZ: %f\n", data[5]);

    if (m_data_memory[m_data_idx].size() > 1048576)
      printf("data size [MiB]:\n\t%.3f\n", m_data_memory[m_data_idx].size() / 1048576.0);
    else if (m_data_memory[m_data_idx].size() > 1024)
      printf("data size [KiB]:\n\t%.2f\n", m_data_memory[m_data_idx].size() / 1024.0);
    else
      printf("data size [B]:\n\t%d\n", m_data_memory[m_data_idx].size());

    printf("\n");
    fflush(stdout);
  } else if (m_debug == 1 && abs(t->tm_min - last_min) >= 5) {
    printf("[PLATYPUS] %d-%d-%d %2d:%2d | ", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min);
    if (m_data_memory[m_data_idx].size() > 1048576)
      printf("%.3f MiB\n", m_data_memory[m_data_idx].size() / 1048576.0);
    else if (m_data_memory[m_data_idx].size() > 1024)
      printf("%.2f KiB\n", m_data_memory[m_data_idx].size() / 1024.0);
    else
      printf("%d B\n", m_data_memory[m_data_idx].size());

    fflush(stdout);

    last_min = t->tm_min;
  }
}

//_______________________________________________________________________________________________________
void platypus::printMenu(int pos) {
  m_dsp->clear();
  m_dsp->print("  Back", 5, 5);

  if (m_wifi_enabled)
    m_dsp->print("  Disable WiFi", 5, 15);
  else
    m_dsp->print("  Enable WiFi", 5, 15);

  if (m_bt_enabled)
    m_dsp->print("  Disable Bluetooth", 5, 25);
  else
    m_dsp->print("  Enable Bluetooth", 5, 25);

  m_dsp->print("  Save RAM Data", 5, 35);
  m_dsp->print("  Display Stats", 5, 45);
  m_dsp->print("  Display Config", 5, 55);
  m_dsp->print(">", 5, 5 + (10 * (pos-1)));
}

//_______________________________________________________________________________________________________
void platypus::printTimer(int sec) {
  m_dsp->print(MENU_TIME - sec, m_dsp->resolution()/2, m_dsp->resolution()-10, true);
}
