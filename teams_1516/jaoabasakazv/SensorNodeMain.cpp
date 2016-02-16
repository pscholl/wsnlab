/*BB****************************************************************************
\file SensorNodeMain.c

\DESCRIPTION:   
       - This File is part of our wsnlab project
       - Reads/manipulate platypus sensors and send over network

\n Copyright (c) wsnlab                           
\n Date   : 12.02.2016
\n Boniface Bassey
\n 
******************************************************************************/

#include <iostream>
#include <sys/types.h>
#include <vector>
#include <iomanip>
#include "./imu_edison.h"
#include "./batgauge_edison.h"
#include "./ldc_edison.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define PORT 8880
#define serverIPv6 "bbbb::fec2:deff:fe30:ad8e"

#define LOOP_FOREVER while(true)

using namespace std;

char time_buf[256];
time_t curtime;
struct tm *loctime;

//Create objects as pointers: imu, battery guage, ldc
imu_edison *m_imu;
batgauge_edison *m_batgauge;
ldc_edison *m_ldc;

//default configuration
int m_i2c_bus = 1; 
uint8_t m_mpu_i2c_addr = MPU_I2C_ADDR;
int m_alert_threshold = 10;

bool m_start_imu = true;
bool m_start_env = true;
bool m_start_bat = true;
bool m_start_ldc = true;
bool success = true;

int m_envID = 0x60;
int m_magID = 0x48;
int m_imuID = 0x71;
int m_batID = 3;

//*************** Accelerometer declarations***************
int tolerance = 3; 			// Sensitivity of the sensor
bool calibrated = false; 	// When accelerometer is calibrated - changes to true 
bool moveDetected = false; 	// When motion is detected - changes to true
	
//Accelerometer limits
float xMin; //Minimum x Value
float xMax; //Maximum x Value
float xVal; //Current x Value

float yMin; //Minimum y Value
float yMax; //Maximum y Value
float yVal; //Current y Value

float zMin; //Minimum z Value
float zMax; //Maximum z Value
float zVal; //Current z Value
//*********************************************************


//******************* Function Prototypes******************
bool printSensorsID(int read, int target);
bool getSensorsID();		//Check for sensors availability
void Initializations();		//Setup IMU, battery gauge
void BatteryGauge();  		//Battery gauge
void readIMU();				//IMU
void getEnvSensors();		//Envornmental Sensors
void printValues();			//Print Accelerometer after calibration
void calibrateAccel();  	//Calibrate Accelerometer
bool checkMotion();			//Check for motion
void INT_HANDLER(int sig);	//Ctrl-C interrupt


//*************Setup IMU, battery gauge, LDC***************
void Initializations()
{
	//Setup IMU
	if (m_start_imu){
		m_imu = new imu_edison(m_i2c_bus, m_mpu_i2c_addr, m_start_env);
		m_imu->setupIMU();
	}

	//Setup battery gauge
	if (m_start_bat){
		m_batgauge = new batgauge_edison();
		m_batgauge->setAlertThreshold(m_alert_threshold);
	}
	
	//Setup ldc
	//if (m_start_ldc){
		//m_ldc = new ldc_edison();	
	//}
}

//********************Get sensors ID**********************
bool getSensorsID()
{		
	cout << "[IMU] ";
	if (!printSensorsID(m_imu->getID(), m_imuID))				//IMU
		success = false;
	
	cout << "[BAT] ";
	if (!printSensorsID(m_batgauge->getVersion(), m_batID))		//Battery gauge
		success = false;	

	cout << "[MAG] ";
	if (!printSensorsID(m_imu->getMagID(), m_magID))			//Compass	
		success = false;
	
	cout << "[ENV] ";
	if (!printSensorsID(m_imu->getEnvID(), m_envID))			//Environmental
		success = false;	

	return success;
}

//*******************Print sensors ID*********************
bool printSensorsID(int read, int target)
{
	cout << "read = " << read << ", expected = " << target;
	if (read == target)
		cout << " : Sensor available.\n";
	else
		cout << " : Sensor unavailable.\n";
	
	return read == target;	
}


//*********************Battery gauge**********************
void BatteryGauge()
{
	if (m_start_bat)
	{	
		cout << "Battery Voltage: " << m_batgauge->getVCell() << "V" << endl;			//Battery voltage
		cout << "State of charge: " << m_batgauge->getSoC() << "%" << endl;				//state of charge
		cout << "Alert Threshold: " << m_batgauge->getAlertThreshold() << "%" << endl;	//Battery alert
	
		if (m_batgauge->getAlertStatus())		//Battery alert status
			cout << "Alert is triggered" << endl;
		else
			cout << "Alert not triggered" << endl;
	}
}

//***********************IMU Sensor************************
void readIMU()
{	
	cout << setw(12) << "Temperature|\t" 
	     << setw(30) << "Accelerometer[m/s^2]|\t" 
	     << setw(30) << "Gyroscope [deg/s]|\t"
	     << setw(30) << "Compass [mGs]|" << endl;
	cout << setw(13) << " [DegC]|\t"
	     << setw(10) << right << "X" << setw(10) << right <<"Y" << setw(10) << right << "Z|\t"
	     << setw(10) << right << "X" << setw(10) << right <<"Y" << setw(10) << right << "Z|\t"
	     << setw(10) << right << "X" << setw(10) << right <<"Y" << setw(11) << right << "Z|\n";

	vector<float> data = m_imu->toReadable(m_imu->readRawIMU());
	float mx, my, mz;
       	m_imu->getCompassData(mx, my, mz);
	
	    cout << setw(12) << data[6] <<"\t"	//Temperature
			 << setw(10) << data[0] << setw(10) << data[1] << setw(10) << data[2] <<"\t"	//Accelerometer
			 << setw(10) << data[3] << setw(10) << data[4] << setw(10) << data[5] <<"\t"	//Gyroscope
			 << setw(10) << mx      << setw(10) << my      << setw(10) << mz 	  << endl; 	//Compass
}
		
//*******************Environmental Sensor********************
void  getEnvSensors()
{
	if (m_start_env && m_start_imu)
	{
		float temp, pressure, humidity;		
		
		cout << setw(18) << "Temperature [DegC]" <<"\t"
		     << setw(14) << "Pressure [hPa]" 	 <<"\t"
	     	 << setw(14) << "Humidity [%RH]" 	 << endl;
   			
		m_imu->getEnvData(temp, pressure, humidity);
	
		cout << setw(18) << temp 	 << "\t"
		     << setw(14) << pressure << "\t"
		     << setw(14) << humidity << "\t" << endl;		
	}
}


//******************************************************************************
// MAIN
//******************************************************************************
int main (int argc, char** argv)
{		
	int sock;
	socklen_t clilen;
	struct sockaddr_in6 server_addr, client_addr;
	char buffer[1024];
	char addrbuf[INET6_ADDRSTRLEN];

	// create a DGRAM (UDP) socket in the INET6 (IPv6) protocol 
	sock = socket(PF_INET6, SOCK_DGRAM, 0);

	if (sock < 0) {
		perror("creating socket");
		exit(1);
	}

	// create server address: where we want to send to 
	
	// clear it out 
	memset(&server_addr, 0, sizeof(server_addr));

	// it is an INET address 
	server_addr.sin6_family = AF_INET6;

	// the server IP address, in network byte order 
	//inet_pton(AF_INET6, argv[1], &server_addr.sin6_addr);
	inet_pton(AF_INET6, serverIPv6, &server_addr.sin6_addr);

	// the port we are going to send to, in network byte order 
	// get the port number from the IP address
	char *sep_ip; 
	char last_segment[20];
	
	//get the last digits
	printf("IPv6 address of the server: %s\n", argv[1]);
	sep_ip = strtok(argv[1], "::");
	while (sep_ip != NULL) {                        
		//copy the last segment of the IP address to an array
		strcpy(last_segment, sep_ip);
		sep_ip = strtok(NULL,":");
	}

	//convert it to an integer
	int port_con;

	//unsigned char byte;
	sscanf(last_segment,"%x",&port_con);
	//byte = port_con & 0xFF;
	 
	//if (port_con >= 1000) {
		port_con = (port_con/1000);
		//port_con = (port_con/1000) + 64000 + port;
	//}

	printf("Connecting through port: %d\n", port_con);
	//server_addr.sin6_port = htons(port_con + 64000);
	server_addr.sin6_port = htons(PORT);

// ******************// *******************// *****************
// ******************// *******************// *****************
	float temp, pressure, humidity;	
	char sendbuffer[1024];
	//char ID_Test[10] = {0};
	//char ENV_H[10] = {0};	
	//char ENV_T[10] = {0};
	//char IMU_xyz[10] = {0};
	//char BAT_ALERT[12] = {0};
	//float BAT_Vol;
	//int BAT_SoC;
	//int BAT_A_T;
			
	memset(sendbuffer,0,1024);

	//setup IMU, battery gauge, lDC
	Initializations();

	//Get Sensors ID	
	printf("\n-------------------------------\n");
	success = getSensorsID();
	if (success){
		//strcpy(ID_Test, "ID_P");
		printf("Sensors ID Test...PASSED\n");
	}else{ 
		//strcpy(ID_Test, "ID_F");
		printf("Some sensors ID Test...FAILED\n");
	}
	printf("\n-------------------------------\n");

	//Battery gauge
	BatteryGauge();
	printf("\n-------------------------------\n");
	
	//Environmental sensor
	getEnvSensors();
	printf("\n-------------------------------\n");
	
	//IMU
	readIMU();
	printf("\n-------------------------------\n");
	
	//initialise and recalibrate the Accelerometer limits.
	calibrateAccel();
		

	LOOP_FOREVER
	{					
		do{	
			//Program termination request
			signal(SIGINT,INT_HANDLER);	
			
			//Environmental sensor--- Get humidity reading
			m_imu->getEnvData(temp, pressure, humidity);
			
			//Humidity Sensor
			if (humidity > 45){	
				break;
			}
			
			// Once the accelerometer is calibrated - check for movement 
			if(checkMotion()){
				moveDetected = true;
				//strcpy(IMU_xyz, "VIB");
				break;
			} 
			else{
				//strcpy(IMU_xyz, "NVIB");
				moveDetected = false;
			}
			 
			//Temperature	
			if(temp > 35){
				break;
			}
			usleep(100000);	//wait for 100ms
		}while((humidity < 45)||(moveDetected == false));

	//Time of event:
		curtime = time (NULL);	// Get the current time.  		
  		loctime = localtime (&curtime);	// Convert it to local time representation. 
  		strftime (time_buf, 12, "%I:%M:%S%p", loctime);
		
	//Battery Gauge		
		//BAT_Vol = m_batgauge->getVCell();
		//BAT_SoC = m_batgauge->getSoC();
		//BAT_A_T = m_batgauge->getAlertThreshold();		
		if (m_batgauge->getAlertStatus()){
			//strcpy(BAT_ALERT, "BAT_ALRT");
			strcpy(sendbuffer, "D");			
			printf("%s | Battery Alert\n",time_buf);
			
			//now send a datagram 
			if (sendto(sock, sendbuffer, sizeof(sendbuffer), 0,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
				perror("sendto failed");
				exit(4);
			}
		usleep(5000000);
		}else{
			//strcpy(BAT_ALERT, "N_BAT_ALRT");
		}
			
	//Humidity Sensor
		if (humidity > 45){
			//strcpy(ENV_H, "BLOW");
			strcpy(sendbuffer, "B");
			printf("%s | BLOW\n",time_buf);
			
			//now send a datagram 
			if (sendto(sock, sendbuffer, sizeof(sendbuffer), 0,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
				perror("sendto failed");
				exit(4);
			}
		usleep(1000000);
		}else{
			//strcpy(ENV_H, "NBLOW");
		}
		
	//Accelerometer		
		if(moveDetected == true){
			strcpy(sendbuffer, "A");
			printf("%s | VIBRATION\n",time_buf);

			//now send a datagram 
			if (sendto(sock, sendbuffer, sizeof(sendbuffer), 0,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
				perror("sendto failed");
				exit(4);
			}	
		usleep(1000000);
		}else{		
			//strcpy(IMU_xyz, "NVIB");
		}
		
	//Temperature	
		if(temp > 35){
			//strcpy(ENV_T, "HOT");
			strcpy(sendbuffer, "C");
			printf("%s | HOT\n",time_buf);
			
			//now send a datagram 
			if (sendto(sock, sendbuffer, sizeof(sendbuffer), 0,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
				perror("sendto failed");
				exit(4);
			}	
		usleep(1000000);
		}else{		
			//strcpy(ENV_T, "COLD");
		}


	//Prepare message buffer to send
		//sprintf(sendbuffer, "%s | %s | %.4fV | %d% | %d% | %s | %s | %s | %.4f |", 
		//	time_buf, ID_Test, BAT_Vol, BAT_SoC, BAT_A_T, BAT_ALERT, ENV_H, IMU_xyz, temp);

		//sprintf(sendbuffer, "%s | %s | %s | %s |", BAT_ALERT, ENV_H, ENV_T, IMU_xyz);
		//fputs (sendbuffer, stdout);
	
	//now send a datagram 
/*		if (sendto(sock, sendbuffer, sizeof(sendbuffer), 0,
			 (struct sockaddr *)&server_addr,
				sizeof(server_addr)) < 0) {
			perror("sendto failed");
			exit(4);
		}		
*/

		do{
			//Environmental sensor--- Get humidity reading
			m_imu->getEnvData(temp, pressure, humidity);			
													
			if (temp > 35)
				printf("...Waiting for Temperature to cool down...\n");
			
			if (humidity > 45)
				printf("...Waiting for Blow to die out...\n");
			
			usleep(5000000);
		}while((humidity > 45) || (temp > 35));
		
		printf("DONE...READY FOR NEXT OPERATION.\n");
	}		 
 
}

//Interrupt Handler on Ctrl-C
void  INT_HANDLER(int sig)
{
    char  c;

    signal(sig, SIG_IGN);
    printf("Did you hit Ctrl-C?\n"
        "Do you really want to quit? [y/n] ");
     	
	c = getchar();
     	if (c == 'y' || c == 'Y')
	{
        	if (success)
			printf("All ID [TEST] passed\n");
		else 		
			printf("Some ID [TEST] failed\n");

		if (m_start_bat)
			delete m_batgauge;
		if (m_start_ldc)
			delete m_ldc;
		if (m_start_imu)
			delete m_imu;				   
		exit(0);
	}
     	else{
          	signal(SIGINT, INT_HANDLER);
     		getchar(); // Get new line character
	}
}

// Function used to calibrate the Accelerometer
void calibrateAccel()
{
 	// reset Detection
 	moveDetected=false;
 
 	//initialise x,y,z variables
	vector<float> data = m_imu->toReadable(m_imu->readRawIMU());
	xMin = data[0];
	xMax = data[0];
	yMin = data[1];
	yMax = data[1];
	zMin = data[2];
	zMax = data[2];
	 
 	// Calibration sequence initialisation sound - 3 seconds before calibration begins
 	printf("\nCalibrating Accelerometer Sensor...\n");
 
 	//calibrate the Accelerometer (should take about 0.5 seconds)
 	for (int i=0; i<50; i++)
	{  
 		vector<float> data = m_imu->toReadable(m_imu->readRawIMU());

 		// Calibrate X Values
 		xVal = data[0];
 		if(xVal>xMax){
 			xMax=xVal;
 		}else if (xVal < xMin){
 			xMin=xVal;
 		}

 		// Calibrate Y Values
 		yVal = data[1];
 		if(yVal>yMax){
 			yMax=yVal;
 		}else if (yVal < yMin){
 			yMin=yVal;
 		}

 		// Calibrate Z Values
 		zVal = data[2];
 		if(zVal>zMax){
 			zMax=zVal;
 		}else if (zVal < zMin){
 			zMin=zVal;
 		}

 		//Delay 10msec between readings
 		usleep(10000);
 	}
 	//End of calibration.
	
	printf("Accelerometer is Calibrated.\n");
 	printValues();
 	calibrated = true;
}


// Prints the Sensor limits identified during Accelerometer calibration.
void printValues()
{
 	printf("xMin= %f\t", xMin);
 	printf(", xMax= %f\n", xMax);
 	printf("yMin= %f\t", yMin);
 	printf(", yMax= %f\n", yMax);
 	printf("zMin= %f\t", zMin);
 	printf(", zMax= %f\n", zMax);
	printf("\n-------------------------------\n");
}

//Function used to detect motion. Tolerance variable adjusts the sensitivity of movement detected.
bool checkMotion()
{
	bool tempB=false;

 	vector<float> data = m_imu->toReadable(m_imu->readRawIMU());

 	xVal = data[0];
 	yVal = data[1];
 	zVal = data[2];
 
 	if(xVal >(xMax+tolerance)||xVal < (xMin-tolerance)){
 		tempB=true;
 		printf("Motion detected in X-direction\n");
 	}
 
	if(yVal >(yMax+tolerance)||yVal < (yMin-tolerance)){
	 	tempB=true;
	 	printf("Motion detected in Y-direction\n");
	}
 
 	if(zVal >(zMax+tolerance)||zVal < (zMin-tolerance)){
 		tempB=true;
 		printf("Motion detected in Z-direction\n");
 	}
   return tempB;
}

