/*
* Author: Mareike Höchner
* MCU programm
* !sensors have to be connected to I2C-6!
*
*/

#include "mcu_api.h"
#include "mcu_errno.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "imu_mcu.h"
#include "batgauge_mcu.h"
#include "ldc_mcu.h"
#include "display_mcu.h"

#define false   0
#define true    1

// default config
uint8_t m_mpu_address = 0x68;
int m_alert_threshold = 4;
int samplerate_mpu = 25; // Default value of the sample rate in [Hz]
int c_hands = 2; // Amount of hands
_Bool wait_for_atom = true;
_Bool m_start_imu = true; // without the imu there is no interrupt - no sensors are read if this variable is false
_Bool m_start_ldc = true;
_Bool m_start_env = true;
_Bool m_start_mag = true; // Compass
_Bool m_start_bat = true;
_Bool m_start_dsp = true;
_Bool m_enabled = false;
_Bool m_irq_reg = false;
_Bool m_sending = false;
_Bool m_missed_timeset = true;
_Bool m_data_compression = false;

// variables to store sensor data temporary
_Bool batteryAlert; // sensor: BAT
int soc; // sensor: BAT
_Bool charging; // 1 if battery is charging
int16_t ldc[2]; // sensor: LDC
int16_t temp; // Temperature (from MPU)
int32_t comp_T; // sensor: BME
uint32_t comp_P; // sensor: BME
uint32_t comp_H; // sensor: BME
int16_t mag_X; // Compass, sensor: MPU
int16_t mag_Y; // Compass, sensor: MPU
int16_t mag_Z; // Compass, sensor: MPU

// BUFFER for MPU data:
// buffer pointer
uint16_t rdptr;
uint16_t wrptr;
uint16_t content;
// buffer variables;
static int const buflen = 32768; // >power of two<
int16_t data[32768]; // data[buflen];

// BUFFER compression Variables
int16_t escape_character = -32768; // defines the escape character for the compression algorithm
uint16_t character_amount = 0;	// buffers the amount of characters which are similar
int16_t character[6];	// puffers the last characters
uint8_t accel_max_deviation = 5; // defines the maximum deviation to the value before a acceleration value can have until it will be written as a new value to the databuffer
uint8_t gyro_max_deviation = 10;	// defines the maximum deviation to the value before a gyroscope value can have until it will be written as a new value to the databuffer

// Clock Variables
struct tm times;
unsigned long last_time_ms;
unsigned long last_set_time_ms;
unsigned long time_ms_now;
unsigned long dif;

//_______________________________________________________________________________________________________
// this is really stupid, but atoi and strtol crash the MCU and sscanf seems to be unavailable (compile errors from stdio.h)
int string_to_int(char* string) {
	//char nums[10] = {'0','1','2','3','4','5','6','7','8','9'};
	int len = 0;
	int integer = 0;
	int i;

	len = strlen(string);
	for(i=0;i<len;i++) {
		integer = integer*10 + (string[i]-'0');
	}
	return integer;
}

//_______________________________________________________________________________________________________
void bufferWrite(int16_t value) {
	if(content < buflen) {
		data[wrptr++ & (buflen-1)] = value;
		content++;
	} else {
		debug_print(DBG_ERROR, "[MAIN] write more data than possible.\n");
	}
}

//_______________________________________________________________________________________________________
int16_t bufferRead() {
	int16_t value = 0;
	if(content > 0) {
		value = data[rdptr++ & (buflen-1)];
		content--;
	} else {
		debug_print(DBG_ERROR, "[MAIN] read more data than available.\n");
	}
	return value;
}

//_______________________________________________________________________________________________________
int imu_irq(int req) {
	if (!m_enabled) {
		return IRQ_HANDLED;
	}

	debug_print(DBG_DEBUG, "[IRQ] handling interrupt.\n");

	static int irq_cnt;
	irq_cnt++;
	uint8_t is = getIntStatus();

	/* DISABLED FOR NOW
	char buf[64];
	int len = mcu_snprintf(buf, 9, "\nwom_int\n");

	if(hasWOMInt(is))
		debug_print(DBG_INFO, "[IRQ] WOM interrupt.\n");

	// if a wake on motion interrupt has been triggered, pull up passthrough gpio
	if(hasWOMInt(is)) {
		gpio_write(46, 1);
		if (!m_sending)
			host_send((unsigned char*)buf, len);
	} else {
		gpio_write(46, 0);
	}
	*/

	// If the Interrupt was triggerd by RAW RDY:
	// read the new raw data and save them in the MCU buffer
	if(hasRawRdyInt(is)) {
		int16_t tmp[6];
		readRawIMU(tmp);

		// Save the data with compression
		if(m_data_compression) {
			// Make sure that the escape character does not appear in the data.
			if(tmp[0] == escape_character) {tmp[0]++;}
			if(tmp[1] == escape_character) {tmp[1]++;}
			if(tmp[2] == escape_character) {tmp[2]++;}
			if(tmp[3] == escape_character) {tmp[3]++;}
			if(tmp[4] == escape_character) {tmp[4]++;}
			if(tmp[5] == escape_character) {tmp[5]++;}
			// In the beginning no value is in the pipeline. Save the current value to compare it with the next one.
			if(character_amount == 0) {
				character_amount++;
				character[0] = tmp[0];
				character[1] = tmp[1];
				character[2] = tmp[2];
				character[3] = tmp[3];
				character[4] = tmp[4];
				character[5] = tmp[5];
			} else {
				// If the amount variable is not full and the current value is nearly the same to the last, just increase the amount variable.
				if(character_amount < 65535 &&
				abs(tmp[0]-character[0]) <= accel_max_deviation &&
				abs(tmp[1]-character[1]) <= accel_max_deviation &&
				abs(tmp[2]-character[2]) <= accel_max_deviation &&
				abs(tmp[3]-character[3]) <= gyro_max_deviation &&
				abs(tmp[4]-character[4]) <= gyro_max_deviation &&
				abs(tmp[5]-character[5]) <= gyro_max_deviation) {
					character_amount++;
				}
				// If the amount variable is full or the current value is different to the last one, the saved values will be written to the databuffer.
				else {
					// If there is just one value saved, write it to the databuffer.
					if(character_amount == 1) {
						bufferWrite(character[0]);
						bufferWrite(character[1]);
						bufferWrite(character[2]);
						bufferWrite(character[3]);
						bufferWrite(character[4]);
						bufferWrite(character[5]);

						// TEST debugprint
						//debug_print(DBG_INFO, "		%d	%d	%d	%d	%d	%d\n", character[0], character[1], character[2], character[3], character[4], character[5]);
					}
					// If there are more than one value saved, write the escape character, the amount of same values and the value to the databuffer.
					else {
						bufferWrite(escape_character);
						bufferWrite(character_amount);
						bufferWrite(character[0]);
						bufferWrite(character[1]);
						bufferWrite(character[2]);
						bufferWrite(character[3]);
						bufferWrite(character[4]);
						bufferWrite(character[5]);

						// TEST debugprint
						//debug_print(DBG_INFO, "%d x		%d	%d	%d	%d	%d	%d\n", character_amount, character[0], character[1], character[2], character[3], character[4], character[5]);
					}
					// The old value is sent. The new one can be saved.
					character_amount = 1;
					character[0] = tmp[0];
					character[1] = tmp[1];
					character[2] = tmp[2];
					character[3] = tmp[3];
					character[4] = tmp[4];
					character[5] = tmp[5];
				}
			}
		}

		// Save the data without compression
		else {
			int i;
			for(i=0; i< 6; i++) {
				bufferWrite(tmp[i]);
			}
		}
	}

	// Check the Battery Alarm State and all the other sensors every 1000th interrupt
	// every sensor in another interrupt
	if(irq_cnt == 996) {
		// read Temperature from MPU
		temp = readRawTemp();
	}
	if(irq_cnt == 997) {
		// read LDC data
		if(m_start_ldc) {
			getADC(ldc);
		}
	}
	if(irq_cnt == 998) {
		// read ES data from BME sensor
		if(m_start_env) {
			getEnvData(&comp_T, &comp_P, &comp_H);
		}
	}
	if(irq_cnt == 999) {
		// read Magnetometer(MPU) Compass data
		if(m_start_mag) {
			getCompassData(&mag_X, &mag_Y, &mag_Z);
		}
	}
	if(irq_cnt >= 1000) {
		irq_cnt = 0;

		if(m_start_bat) {
			soc = getSoC();
			if(getAlertStatus()) {
				batteryAlert = 1;
				// stop IMU - stops the interrupts as well
				stopIMU();
			}
		}
	}

	return IRQ_HANDLED;
}

//_______________________________________________________________________________________________________
void parseTime(const char * pt) {
	// parse the current time stamp from received buffer
	// sscanf would do this in one line, but seems to be unavailable (compile errors from stdio.h)
	//const char * pt = buf;
	char * pos = strchr(pt, ':');
	char h[3] = "00";
	char m[3] = "00";
	char s[3] = "00";

	h[0] = pos[1];
	h[1] = pos[2];
	h[2] = '\0';
	pos = strchr(pos+1,':');
	m[0] = pos[1];
	m[1] = pos[2];
	m[2] = '\0';
	pos = strchr(pos+1,':');
	s[0] = pos[1];
	s[1] = pos[2];
	s[2] = '\0';

	times.tm_hour = string_to_int(h);
	times.tm_min = string_to_int(m);
	times.tm_sec = string_to_int(s);

	m_missed_timeset = false;
	last_set_time_ms = time_ms();

	debug_print(DBG_INFO, "[MCU] time set to: %d:%d:%d\n", times.tm_hour, times.tm_min, times.tm_sec);
}

//_______________________________________________________________________________________________________
void parseRate(const char * pt) {
	// parse the sample rate from the received buffer
	char * pos = strchr(pt, ':');
	char r[5] = "0000";
	int i;

	for(i=0; i<4; i++) {
		r[i] = pos[i+1];
		if(pos[i+1] == ':') {
			r[i] = '\0';
		}
	}
	r[4] = '\0';

	samplerate_mpu = string_to_int(r);
	debug_print(DBG_INFO, "[MCU] sample rate set to: %d\n", samplerate_mpu);
}

//_______________________________________________________________________________________________________
int atomCommand() {
	char buf[64];
	int len = host_receive((unsigned char *)buf, 64);
	// if received something
	if(len > 0) {
		// if received time command
		if(strncmp(buf, "mcu_time", 8) == 0) {
			parseTime(buf);
			return 1;
		}
		// if received start command
		else if(strncmp(buf, "mcu_start", 9) == 0) {
			debug_print(DBG_INFO, "[MCU] start.\n");
			// Set the Sample rate if sent
			if(strncmp(buf+10, "mcu_rate", 8) == 0) {
				parseRate(buf);
			}
			// Tell Atom that MCU started and send time stamp
			len = mcu_snprintf(buf, 44, "[MCU] start %i\n", time_ms());
			host_send((unsigned char*)buf, len);
			m_enabled = true;
			return 2;
		}
		// if received stop command
		else if(strncmp(buf, "mcu_stop", 8) == 0) {
			debug_print(DBG_INFO, "[MCU] stop.\n");
			//Tell Atom that MCU stopped
			host_send((unsigned char*)"[MCU] stop\n", 11);
			m_enabled = false;
			return 3;
		} else {
			debug_print(DBG_ERROR, "received %s\n", buf);
			len = 0;
			return -1;
		}
	}
	return 0;
}

//_______________________________________________________________________________________________________
void updateChargingState() {
	static _Bool ccharging = 0;
	// Read from gpio 45 the charging state
	charging = !(_Bool)gpio_read(45);
	if (ccharging != charging) {
		if (charging) {
			debug_print(DBG_INFO, "[MCU] start charging.\n");
			host_send((unsigned char*)"charging_start\n", 16);
		} else {
			debug_print(DBG_INFO, "[MCU] stop charging.\n");
			host_send((unsigned char*)"charging_stop\n", 15);
		}
		ccharging = charging;
	}
}

//_______________________________________________________________________________________________________
void updateTime() {
	time_ms_now = time_ms();

	if(time_ms_now > last_time_ms) {
		dif = time_ms_now-last_time_ms;
	}
	else {
		dif = -1;
		dif -= last_time_ms;
		dif += time_ms_now;
	}

	times.tm_sec += dif/1000;
	if (times.tm_sec >= 60) {
		times.tm_min += times.tm_sec/60;
		if (times.tm_min >= 60) {
			times.tm_hour += times.tm_min/60;
			times.tm_hour = times.tm_hour % 24;
			times.tm_min = times.tm_min % 60;
		}
		times.tm_sec = times.tm_sec % 60;
	}

	last_time_ms = time_ms_now - (dif%1000);
}

//_______________________________________________________________________________________________________
void mcu_main() {
	char buf[128];
	int len;

	START:

	rdptr = 0;
	wrptr = 0;
	content = 0;

	// wait for Atom command to start
	if(wait_for_atom) {
    	while (atomCommand() != 2) {
    		mcu_sleep(10);
    	}
	}

	//Initialize Display
	if(m_start_dsp) {
		last_time_ms = time_ms();
		last_set_time_ms = time_ms();
		res = 128;
		init_display(c_hands);
		clear_display();
		print_string("WELCOME TO", res/2, res/2-5, true, true);
		print_string("PLATYPUS", res/2, res/2+5, true, true);
		flush();
		mcu_sleep(100);
	}

	// setup Batgauge
	if(m_start_bat) {
		setupBatgauge();
		setAlertThreshold(m_alert_threshold);
		soc = getSoC();
	}

	// setup LDC
	if(m_start_ldc) {
		setupLDC();
	}

	// setup IMU
	if(m_start_imu) {
		setupIMU(samplerate_mpu, m_mpu_address, false, m_start_env, m_start_mag);
		if (!m_irq_reg) {
			// imu interrupt gpio
			gpio_setup(14, 0);
			// register isr for IMU
			// Once the interrupt is started, do not use I2C anymore (only in the interrupt!!!)
			gpio_register_interrupt(14, 0, imu_irq); //mode: 0 = falling edge interrupt
			m_irq_reg = true;
			debug_print(DBG_INFO, "[IRQ] registered.\n");
		}
		// WoM passthrough gpio
		// mirror any WoM interrupt so the atom can see it
		gpio_setup(46, 1);
	}

	while(1) {
		// sleep 1000 milliseconds (=100 mcu ticks)
		mcu_sleep(100);

		// update charging state
		updateChargingState();

		// update time (converts time_ms() to hour, minute and second variables)
		updateTime();

		// Print Clock
		if(m_start_dsp && DSPChanges(&times, soc, charging)) {
			clear_display();
			analogClock(&times);
			batteryCharge(soc, charging);
			warning(m_missed_timeset);
	    	flush();
		}

		// if MCU buffer is 85% full, send data to the atom
		if(content >= buflen*85/100) {
			debug_print(DBG_INFO, "[MAIN] send data to the Atom.\n");

			// send data_start and time stamp to the Atom
			m_sending = true;
   			int datalen = content/126;
			len = mcu_snprintf(buf, 128, "data_start %i\nTimecode1: %i\n", datalen*252, time_ms());
			host_send((unsigned char*)buf, len);

			mcu_sleep(500);

			// send accel and gyro data to the Atom
			len = mcu_snprintf(buf, 128, "IMU:\n");
			host_send((unsigned char*)buf, len);
   			for(; datalen>0; datalen--) {
   				int16_t databuf[126];
   				int j;
   				for(j=0; j<126; j++) {
   					databuf[j] = bufferRead();
   				}
   				host_send((unsigned char*)&databuf[0], 252);
   				mcu_sleep(1);
   			}
   			host_send((unsigned char*)"\n", 1);

  			// send Temperature(MPU) to the Atom
   			len = mcu_snprintf(buf, 128, "Temperature: %i\n", temp);
   		   	host_send((unsigned char*)buf, len);

  			// send LDC values to the Atom
   			if(m_start_ldc) {
   				len = mcu_snprintf(buf, 128, "ADC 0: %i\nADC 1: %i\n", ldc[0], ldc[1]);
   				host_send((unsigned char*)buf, len);
   			}

   			// send Magnetometer(MPU) to the Atom
   			if(m_start_mag) {
   				len = mcu_snprintf(buf, 128, "Compass: %i, %i, %i\n", mag_X, mag_Y, mag_Z);
   				host_send((unsigned char*)buf, len);
   			}

   			// send Temperature, Pressure, Humidity (BME) to the Atom
   			if(m_start_env) {
   				len = mcu_snprintf(buf, 128, "Temperature(BME): %d\nPressure(BME): %i\nHumidity(BME): %i\n", comp_T, comp_P, comp_H);
   				host_send((unsigned char*)buf, len);
   			}

   			// send SoC (state of charge) to the Atom
   			if(m_start_bat) {
   				len = mcu_snprintf(buf, 128, "State of Charge: %i\n", soc);
   				host_send((unsigned char*)buf, len);
   			}

			// send time stamp to the Atom
   			len = mcu_snprintf(buf, 128, "Timecode2: %i\ndata_end\n\n", time_ms());
   			host_send((unsigned char*)buf, len);
   			m_sending = false;

   			// no time set after 10 minutes -> something probably went wrong on the Atom
   			// do this here so it doesn't falsely trigger when compressing
   			if (time_ms() - last_set_time_ms > 1000*60*10)
   				m_missed_timeset = true;
		}

		// Read atom commands
		int command = atomCommand();
		// Check for mcu stop signal
		if(command == 3) {
			stopIMU();
			goto START; // jump back to beginning of main, to wait for next start command
		}

		// Check the Battery Alarm State (set by the interrupt)
   		if(batteryAlert) {
   			m_enabled = false;
   			break;
   		}
	}
   	// TODO stop all sensors
}
