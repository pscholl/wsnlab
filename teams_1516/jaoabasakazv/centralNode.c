/*
  Firmware of the central node in the WSN
  Jorge Saez
  WSN Laboratory
  Uni Freiburg
  Freiburg, 25.01.2015
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "localizationList.h"
#include "udpClient6.h"
#include "splitBuffer.h"


#define PORT_REC 8880
#define PORT_SEND 8881
#define MESSAGE "hello"
#define MESSAGE2 "TO YOUR RIGHT"

#define MESSAGE3 "GO LEFT"
#define MESSAGE4 "COME FROM RIGHT"
#define MESSAGE5 "GO RIGHT"
#define MESSAGE6 "COME FROM LEFT"
//define number of nodes in the network

#define NODES_NUMBER 5
#define ONGOING 1
#define NOTHING 0
#define ENABLED 1
#define DISABLED 0
  #define LEFTMOST 1



//**************Auxiliary Functions************************************
int calc_furthest(int sensorPosition);
void set_payload(char *sendbuffer, int ongoing_process, int finalDestination, char motionCommand, int enableFigure, int local_done);
void Hot_Event(int hot_node,int sock);

//**********GLOBAL VARS***************************************
int figure_position = 0;//the figure is initialized in the leftmost sensor
struct arraylist list;

int calc_furthest(int sensorPosition) {
	int goal;
	if(sensorPosition <= NODES_NUMBER/2) {
		goal = NODES_NUMBER-1;

	} else {
		goal = 0;
	}
	return goal;

}

void set_payload(char *sendbuffer, int ongoing_process, int finalDestination, char motionCommand, int enableFigure, int local_done) {
	char timeBuffer[256];
	char ID_Test[10] = {0};
	char ENV_H[10] = {0};	
	char IMU_xyz[10] = {0};
	char BAT_ALERT[12] = {0};
	char display_info[256];//*************************information of the display!!!!!!!!!!!!!!!!!
	float BAT_Vol = 0;
	int BAT_SoC = 0;
	int BAT_A_T = 0;
	float temp;
	//int ongoing_process = 0;//****
	//int finalDestination;
	//char motionCommand;

	sprintf(sendbuffer, "%s|%s|%.4fV|%d|%d|%s|%s|%s|%.4f|%s|%d|%d|%d|%s ", 
			timeBuffer, ID_Test, BAT_Vol, BAT_SoC, BAT_A_T, BAT_ALERT, ENV_H, IMU_xyz, temp, display_info, enableFigure,
			ongoing_process, finalDestination, motionCommand);
}
//**************
void Hot_Event(int hot_node,int sock)
{
	socklen_t clilen;
	struct sockaddr_in6 client_addr;
	char buffer[1024];

	int furthest = calc_furthest(hot_node);
	
	if(furthest == 0)
	{
		while(figure_position!=furthest)
		{
			// DELETE figure from current position  
			char* AdrToSend1= (char*) malloc(25*sizeof(char));
			strcpy(AdrToSend1, list.data[figure_position]);
     		udpClient6(AdrToSend1, MESSAGE3,PORT_SEND);
			//wait for ACK that figure went off
			printf("waiting for a datagram...\n");
			clilen = sizeof(client_addr);
			if (recvfrom(sock, buffer, 1024, 0,
				 (struct sockaddr *)&client_addr,
				 &clilen) < 0) {
					perror("recvfrom failed");
					exit(4);
		    }
			//SHOW figure on the left node
			char* AdrToSend2= (char*) malloc(25*sizeof(char));
			strcpy(AdrToSend2, list.data[figure_position-1]);
     		udpClient6(AdrToSend2, MESSAGE4,PORT_SEND);
			//wait for ACK that figure appeared
			printf("waiting for a datagram...\n");
			clilen = sizeof(client_addr);
			if (recvfrom(sock, buffer, 1024, 0,
				 (struct sockaddr *)&client_addr,
				 &clilen) < 0) {
					perror("recvfrom failed");
					exit(4);
		    }
			figure_position=figure_position-1;
		}
	}
	else
	{
		while(figure_position!=furthest)
		{// DELETE figure from current position  
			char* AdrToSend3= (char*) malloc(25*sizeof(char));
			strcpy(AdrToSend3, list.data[figure_position]);
     		udpClient6(AdrToSend3, MESSAGE5,PORT_SEND);
			//wait for ACK that figure went off
			printf("waiting for a datagram...\n");
			clilen = sizeof(client_addr);
			if (recvfrom(sock, buffer, 1024, 0,
				 (struct sockaddr *)&client_addr,
				 &clilen) < 0) {
					perror("recvfrom failed");
					exit(4);
		    }
			//SHOW figure on the right node
			char* AdrToSend4= (char*) malloc(25*sizeof(char));
			strcpy(AdrToSend4, list.data[figure_position+1]);
     		udpClient6(AdrToSend4, MESSAGE6,PORT_SEND);
			//wait for ACK that figure appeared
			printf("waiting for a datagram...\n");
			clilen = sizeof(client_addr);
			if (recvfrom(sock, buffer, 1024, 0,
				 (struct sockaddr *)&client_addr,
				 &clilen) < 0) {
					perror("recvfrom failed");
					exit(4);
		    }
			figure_position=figure_position+1;
		}
	}
}
int main(void)
{
  int sock;
  socklen_t clilen;
  struct sockaddr_in6 server_addr, client_addr;
  char buffer[1024];
  char addrbuf[INET6_ADDRSTRLEN];

  //***************Fields of the dataframe
  int local_done = 0; //flag to be set when the localization process is done
  //int figure_position = 0;//the figure is initialized in the leftmost sensor
  int routing = 0; //could be a flag that represents a possible routing mode of the central node
  char vibration_event;
  char blow_event;
  char temperature_event;
  int process_done;
  int arrayList_position = 0;
  char displayInfo[256]; 
  char motionCommand;
  int finalDestination;
  int enableFigure = 0;



//********************************************************************************************
  //****************process variables
  int temp_process = 0;
  int blow_process = 0;
  int vibration_process = 0;
  int ongoing_process = 0;

  char sendbuffer[1024];

/* -------- Initializing localization arraylist ----------------- */
	
	arraylist_initial(&list);

  /* create a DGRAM (UDP) socket in the INET6 (IPv6) protocol */
  sock = socket(PF_INET6, SOCK_DGRAM, 0);

  if (sock < 0) {
    perror("creating socket");
    exit(1);
  }

#ifdef V6ONLY
  // setting this means the socket only accepts connections from v6;
  // unset, it accepts v6 and v4 (mapped address) connections
  { int opt = 1;
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) < 0) {
      perror("setting option IPV6_V6ONLY");
      exit(1);
    }
  }
#endif

  /* create server address: this will say where we will be willing to
     accept datagrams from */

  /* clear it out */
  memset(&server_addr, 0, sizeof(server_addr));

  /* it is an INET6 address */
  server_addr.sin6_family = AF_INET6;

  /* the client IP address, in network byte order */
  /* in this example we accept datagrams from ANYwhere */
  server_addr.sin6_addr = in6addr_any;


  
   printf("Connected to Port: %d\n", PORT_REC);


  //************************************************************************************************
  /* the port we are going to listen on, in network byte order */
  server_addr.sin6_port = htons(PORT_REC);

  /* associate the socket with the address and port */
  if (bind(sock, (struct sockaddr *)&server_addr,
	   sizeof(server_addr)) < 0) {
    perror("bind failed");
    exit(2);
  }

  while (1) {


	char *incomingIp = NULL; //IP address of the sending node

	int index_list1 = 0;
  
  //int localization_flag = ";
  
	



  /* now wait until we get a datagram */
  // we use the same port for every possible connection
    printf("waiting for a datagram...\n");
    clilen = sizeof(client_addr);
    if (recvfrom(sock, buffer, 1024, 0,
		 (struct sockaddr *)&client_addr,
		 &clilen) < 0) {
      perror("recvfrom failed");
      exit(4);
    }
/********************************************************LOCALIZATION***********************************************************************/
//the localization algorithm should be carried out until the max number of nodes in the network is reached
//only vibration packets can trigger the localization algorithm (TODO)
//define vocabulary of the frame(TODO)
//split buffer and extract the accelerometer field to find out if there was a vibration

int field_counter = 0; //counter of the fields of the datagram
char **buffer_array = NULL;  //array which contains the fields
field_counter = splitBuffer(buffer, '|', &buffer_array);
//********************splitting incoming message***********************************
vibration_event = buffer_array[7];
blow_event =buffer_array[6];
temperature_event = buffer_array[8];

//the information regarding the accelerometer is located in the position 7 of the array
if ((local_done == 0) && (vibration_event = "ACC")) {

  //while this process, no figure will be displayed
  	//displayInfo = "NOT";
	
	char * bufferPtr = NULL;

	//store the incoming IP of the node
	incomingIp = inet_ntop(AF_INET6, &client_addr.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
	
	bufferPtr = (char *)malloc(25*sizeof(char));
	strcpy(bufferPtr, incomingIp);	
  	printf("got '%s' from %s\n", buffer, incomingIp);
 
		
	printf("Now going for check and add -------------------\n");
	addToListWithCheck(&list, bufferPtr);
	//**************************************
	arrayList_position = arraylist_get_size(list);
	//**************************************

	printf("------------------------ Printing list after check and add ------------- \n");
	index_list1 = 0;
  	for(index_list1 = 0; index_list1 != 6; index_list1++) {
    		printf("CHECK: %s\n", arraylist_get(list, index_list1));
  	}
    	printf("Size of list = %d \n",arraylist_get_size(list));
		
		
	printf ("Left Ip of - %s - is - %s -\n", incomingIp, getLeftOf (list, incomingIp));
	printf ("Right Ip of - %s - is - %s -\n", incomingIp, getRightOf (list, incomingIp));
	
	int index_list = 0;
  	for(index_list = 0; index_list != 6; index_list++) {
    		printf("CHECK: %s\n", arraylist_get(list, index_list));
  	}
    	printf("Size of list = %d \n",arraylist_get_size(list));
      
      //return message
      //could be returned the actual position of the node in the array list
      //
      printf("sending message back\n");

     // sprintf(sendbuffer, "%s|%d|%s", 
      //local_done, arrayList_position, displayInfo);
      set_payload(sendbuffer, NOTHING, finalDestination, motionCommand, DISABLED, local_done);
      udpClient6(incomingIp, sendbuffer,PORT_SEND);


      //check if the list contains already all the nodes
      if (arraylist_get_size(list) == NODES_NUMBER) {
        local_done = 1;
        printf("Localization is complete\n");


        //make a broadcast transmission to let the nodes know that localization is done
        //*****************************************************************************
        //iterate over the array list and send
        /*int index_broadcast = 0;
        for (index_broadcast = 1; index_broadcast <= NODES_NUMBER;index_broadcast++) {

        	//First, we need to send to the leftmost sensor a notification that allows it to show the figure
        	if (index_broadcast == 1) {
				set_payload(sendbuffer, NOTHING, finalDestination, motionCommand, LEFTMOST, DISABLED, local_done);
				udpClient6(arraylist_get(list, index_broadcast - 1), sendbuffer,PORT_SEND);
        	}

        	set_payload(sendbuffer, NOTHING, finalDestination, motionCommand, index_broadcast, DISABLED, local_done);
        	udpClient6(arraylist_get(list, index_broadcast - 1), sendbuffer,PORT_SEND);


        }*/
        //******************************************************************************

      }


  }
  //*******************************************************END OF LOCALIZATION*************************************************************************/
  //************In this stage the central node needs to determine what to do based on the information received ******************************************/
  //************If an event is triggered in a node, it notices automatically the central node and it will decide****************************************/


  else  {
      //Here we will decide what to do with the incoming packets

  	   //First, we need to send to the leftmost sensor a notification that allows it to show the figure
  		set_payload(sendbuffer, ongoing_process, finalDestination, motionCommand, ENABLED, local_done);

      //First approach: send a message to the right of the incoming IP
      char *incomingIp2 = NULL;
      char *rightIP = NULL;
      incomingIp2 = inet_ntop(AF_INET6, &client_addr.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
      printf("got '%s' from %s\n", buffer, incomingIp2);
      //return ack message

      //the central node should act according to the event which happened in the sensor. 
      //The events which will be taken into account will be vibration (accelerometer) and humidity(blow)
      //we need to store in a variable the actual position of the figure
      //by default the figure will be at the beginning at the leftmost sensor

      //Possible behaviours
      //They need a priority. Once a process is completed, one can move to the next one, i.e., if a blow occurs while the temp. process is being carried out,
      //one needs to wait until this process is finished. 
      //increase of temperature --------------------------------------------->Figure should move to the furthest node in the network
      //blow occurs in a specific sensor ------------------------------------>the figure should move towards that sensor.
      //vibration------------------------------------------------------------>Figure should fall down (is it possible to implement??)
      //

      if ((temperature_event == "HOT") && (arrayList_position == figure_position) && (ongoing_process == 0)) {
      	ongoing_process = 1;



      	
        // and get_index(Incoming_IP == Position_Figure)
      	// we also need to check if other processes are being treated now
      	// send to the incoming sensor a command to move the figure to the right direction
      	// first calculate in which direction the figure needs to move and its final destination
      	char command;
      	int dest = calc_furthest(arrayList_position);
      	if (dest < arrayList_position) {
      		command = "LEFT";
      	} else {
      		command = "RIGHT";
      	}
      	//send to the sensor
      	//
      	set_payload(sendbuffer, NOTHING, dest, command, DISABLED, local_done);
     	udpClient6(incomingIp, sendbuffer,PORT_SEND);





      }
      //blow event
    else if((blow_event == "BLOW") && (ongoing_process == 0)) {


  	}
  	//vibration event
  	else if ((vibration_event == "VIB") && (arrayList_position == figure_position) && (ongoing_process == 0) ) {

  	}
  	//routing mode, the central node only forwards packets
  	//else if((routing == ON) && (ongoing_process == 1)) {



  	//}

      /*
      //calculate the IP which is on the right
      rightIP = getRightOf (list, incomingIp2);
      printf("sending to the right of %s - which is - %s - \n", incomingIp2, rightIP);
	    char* bufferPtr1= (char*) malloc(25*sizeof(char));
	    strcpy(bufferPtr1, rightIP);
      //send message creating a new socket
      //all the information from the central node will be sent through port 8881
	    udpClient6(bufferPtr1, MESSAGE2,PORT_SEND);
	    for(index_list1 = 0; index_list1 != 6; index_list1++) {
    		printf("CHECK: %s\n", arraylist_get(list, index_list1));
  	  }
    	printf("Size of list = %d \n",arraylist_get_size(list));
	*/
      /*if (sendto(sock, MESSAGE2, sizeof(MESSAGE), 0,
               (struct sockaddr *)&client_addr,
         sizeof(client_addr)) < 0) {
      perror("sendto failed");
      exit(5);
      }*/
  }
    //********************************************************
  }
  
  return 0;
}
