#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "sys/cc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT 15000

// Contiki client ports
#define SERVER_PORT1 15001
#define SERVER_PORT2 15002
#define SERVER_PORT3 15003
#define SERVER_PORT4 15004

#define BUFSIZE 1024
#define MAX_CLIENTS 5
static struct tcp_socket socket[MAX_CLIENTS]; //socket, socket1, socket2, socket3, socket4;
static uint8_t inputbuf[MAX_CLIENTS][BUFSIZE];
static uint8_t outputbuf[MAX_CLIENTS][BUFSIZE];

int client_connected[] = {0,0,0,0,0};

enum msg_type{
    server_msg,
    client_msg
};

enum command{
    update,
    response,
    animation,
    display_text
};

enum action{
    start,
    stop
};


PROCESS(tcp_server_process, "Contiki server process");
AUTOSTART_PROCESSES(&tcp_server_process);

char delimiter[] = ":,";
const char *ptr_br = "br";
const char *ptr_quit = "quit\n";
const char *ptr_u = "u";
const char *ptr;


/*---------------------------------------------------------------------------*/
void broadcast(char *message) {
    int i, j;
    int bytesSend = 0;
    for(i = 0; i < MAX_CLIENTS; i++) {
        if(client_connected[i] == 1){
            bytesSend = tcp_socket_send(&socket[i], message, strlen(message));
            for(j = 0; j<BUFSIZE; j++)
                outputbuf[i][j] = '\0';
            printf("%d bytes sent to client %d\n", bytesSend, i);
            printf("Server sent broadcast message: %s to all clients\n", message);
        } else {
            printf("Client %d not connected - no data sent.\n", i);
        }
    }
    for(j = 0; j<BUFSIZE; j++)
        message[j] = '\0';
};



void unicastTo(char *message, int clientNum) {
    int j;
    int bytesSend;
    // Check client connected
    if(client_connected[clientNum] == 1) {
        bytesSend = tcp_socket_send(&socket[clientNum], message, strlen(message));
        for(j = 0; j<BUFSIZE; j++)
            outputbuf[clientNum][j] = '\0';
        printf("%d bytes sent to client %d\n", bytesSend, clientNum);
        printf("Server sent unicast message '%s' to client %d\n",message, clientNum);
    }  else {
        printf("Client %d not connected - no data sent.\n", clientNum);
    }
    for(j = 0; j<BUFSIZE; j++)
        message[j] = '\0';
}



/*---------------------------------------------------------------------------*/
// static tcp_socket_data_callback_t input(struct tcp_socket *s, char *ptr, char *inputptr, int inputdatalen) {
static int input(struct tcp_socket *s, char *ptr, char *inputptr, int inputdatalen) {
    int j;
    char *message;
    message = (char *)malloc (BUFSIZE);
    printf("input %d bytes '%s'\n", inputdatalen, inputptr);   
	ptr = strtok(inputptr, delimiter);
	
	// We got a broadcast
	if (strcmp(ptr, ptr_br) == 0) {
	  ptr = strtok(NULL, delimiter);
	  if(ptr != NULL) {
		printf("Server received broadcast command: '%s' from a client\n", ptr);
		// ptr[inputdatalen-3] = '\0';	// quick fix
		sprintf(message, "1,%s", ptr);
		broadcast(message);
	  }
	  
	// We got a unicast 
	} else if (*ptr == *ptr_u) {  //
	  ptr = ptr + 1;
	  int destClient = atoi(ptr);
	  printf("Server received unicast message for client: %d\n", destClient);
	  // ptr[inputdatalen-1] = '\0';	// quick fix
	  
	  ptr = strtok(NULL, delimiter);
	  if(ptr != NULL) {
        // 1(from client), destination, data
		sprintf(message, "1,%d,%s", destClient, ptr);
		unicastTo(message,destClient);
	  }
	 
	// A client has been disconnected
	} else if (strcmp (inputptr, ptr_quit) == 0) {
        if(s == socket){
		  client_connected[0] = -1;
		  sprintf(message, "\nPlaytpus client has been disconnected.\n");
		  broadcast(message);
		} else if(s == socket+1){
		  client_connected[1] = -1;
		  sprintf(message, "\nClient 1 has been disconnected.\n");
		  broadcast(message);
		} else if(s == socket+2) {
		  client_connected[2] = -1;
		  sprintf(message, "\nClient 2 has been disconnected.\n");
		  broadcast(message);
		} else if(s == socket+3) {
		  client_connected[3] = -1;
		  sprintf(message, "\nClient 3 has been disconnected.\n");
		  broadcast(message);
		} else if(s == socket+5) {
		  client_connected[4] = -1;
		  sprintf(message, "\nClient 4 has been disconnected.\n");
		  broadcast(message);
		}
	} else {
		printf("Invalid message\n");
        //Clear Buffer
        for(j = 0; j<BUFSIZE; j++)
            message[j] = '\0';
	}
	
	//Clear input Buffer manually
	for(j = 0; j<BUFSIZE; j++)
           inputptr[j] = '\0';
	
	// Return the number of data bytes we received, to keep them all in the buffer.
    // return inputdatalen;
	
	// Discard everything / leave nothing in inputbuffer
	return 0;
}


/*---------------------------------------------------------------------------*/
// static tcp_socket_event_callback_t event(struct tcp_socket *s, void *ptr, tcp_socket_event_t ev) {
static void event(struct tcp_socket *s, void *ptr, tcp_socket_event_t ev) {
    char *message;
    printf("event %d\n", ev);
	switch(ev) {
		case TCP_SOCKET_CONNECTED:
			message = (char *)malloc (BUFSIZE);
			if(s == socket){
			  client_connected[0] = 1;
			  sprintf(message, "\nPlaytpus client has been connected.\n");
			  broadcast(message);
			} else if(s == socket+1){
			  client_connected[1] = 1;
			  sprintf(message, "\nClient 1 has been connected.\n");
			  broadcast(message);
			} else if(s == socket+2) {
			  client_connected[2] = 1;
			  sprintf(message, "\nClient 2 has been connected.\n");
			  broadcast(message);
			} else if(s == socket+3) {
			  client_connected[3] = 1;
			  sprintf(message, "\nClient 3 has been connected.\n");
			  broadcast(message);
			} else if(s == socket+4) {
			  client_connected[4] = 1;
			  sprintf(message, "\nClient 4 has been connected.\n");
			  broadcast(message);
			}
			break;
		case TCP_SOCKET_CLOSED:
			// printf("TCP Socket closed");
			// Send disconnect broadcast if client didn't send quit
			message = (char *)malloc (BUFSIZE);
			if(s == socket && client_connected[0]) {
			  client_connected[0] = -1;
			  sprintf(message, "\nPlaytpus client has been disconnected.\n");
			  broadcast(message);
			} else if(s == socket+1 && client_connected[1]) {
			  client_connected[1] = -1;
			  sprintf(message, "\nClient 1 has been disconnected.\n");
			  broadcast(message);
			} else if(s == socket+2 && client_connected[2]) {
			  client_connected[2] = -1;
			  sprintf(message, "\nClient 2 has been disconnected.\n");
			  broadcast(message);
			} else if(s == socket+3 && client_connected[3]) {
			  client_connected[3] = -1;
			  sprintf(message, "\nClient 3 has been disconnected.\n");
			  broadcast(message);
			} else if(s == socket+4 && client_connected[4]) {
			  client_connected[4] = -1;
			  sprintf(message, "\nClient 4 has been disconnected.\n");
			  broadcast(message);
			}
			break;
		default:
			break;
	}
	
    return;
}


/*---------------------------------------------------------------------------*/
void start_listening_sockets() {
    
    // Socket for local tcp connection with platypus client
    tcp_socket_register(&socket[0], NULL,
                        inputbuf[0], BUFSIZE,
                        outputbuf[0], BUFSIZE,
                        input, event);
    tcp_socket_listen(&socket[0], SERVER_PORT);
    printf("socket listening on %d\n", SERVER_PORT);
    
    // Sockets for Contiki connections with all other Contikis
    tcp_socket_register(&socket[1], NULL,
                        inputbuf[1], BUFSIZE,
                        outputbuf[1], BUFSIZE,
                        input, event);
    tcp_socket_listen(&socket[1], SERVER_PORT1);
    printf("Socket1 listening on %d\n", SERVER_PORT1);
    
    tcp_socket_register(&socket[2], NULL,
                        inputbuf[2], BUFSIZE,
                        outputbuf[2], BUFSIZE,
                        input, event);
    tcp_socket_listen(&socket[2], SERVER_PORT2);
    printf("Socket2 listening on %d\n", SERVER_PORT2);
    
    tcp_socket_register(&socket[3], NULL,
                        inputbuf[3], BUFSIZE,
                        outputbuf[3], BUFSIZE,
                        input, event);
    tcp_socket_listen(&socket[3], SERVER_PORT3);
    printf("Socket3 listening on %d\n", SERVER_PORT3);
    
    tcp_socket_register(&socket[4], NULL,
                        inputbuf[4], BUFSIZE,
                        outputbuf[4], BUFSIZE,
                        input, event);
    tcp_socket_listen(&socket[4], SERVER_PORT4);
    printf("Socket4 listening on %d\n", SERVER_PORT4);
}


/*---------------------------------------------------------------------------*/
void init_server() {

	int i;
	for(i = 0; i <  MAX_CLIENTS; i++) {
		client_connected[i] = -1;
	}
	start_listening_sockets();
}



/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tcp_server_process, ev, data)
{
  PROCESS_BEGIN();
	
	init_server();
    
  while(1) {
    PROCESS_PAUSE();      
    
      
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/