/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "contiki-net.h"
#include "sys/cc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

static struct tcp_socket socket;

#define INPUTBUFSIZE 1024
static uint8_t inputbuf[INPUTBUFSIZE];

#define OUTPUTBUFSIZE 1024
static uint8_t outputbuf[OUTPUTBUFSIZE];

PROCESS(tcp_server_process, "Contiki client process");
AUTOSTART_PROCESSES(&tcp_server_process);

static uint8_t get_received;
static int bytes_to_send;

uip_ipaddr_t ipaddr;

// Define your server port here
#define SERVER_PORT         15001
#define SEND_INTERVAL		5 * CLOCK_SECOND


/*---------------------------------------------------------------------------*/
static int input(struct tcp_socket *s, char *ptr, char *inputptr, int inputdatalen) {
  printf("input %d bytes '%s'\n", inputdatalen, inputptr);
  return 0;
}
/*---------------------------------------------------------------------------*/
static void event(struct tcp_socket *s, void *ptr, tcp_socket_event_t ev)
{
  printf("event %d\n", ev);
    
    switch(ev) {
        case TCP_SOCKET_CONNECTED:
            printf("\nClient has been connected.\n");
            break;
        case TCP_SOCKET_ABORTED:
            printf("\nClient has NOT been connected.\n");
            break;
        case TCP_SOCKET_CLOSED:
            printf("TCP Socket closed");
            break;
            
        default:
            //printf("Default: Unknown event");
            break;
    }
  return 0;
}

/*---------------------------------------------------------------------------*/
void start_listening_socket() {
    tcp_socket_register(&socket, NULL,
                        inputbuf, INPUTBUFSIZE,
                        outputbuf, OUTPUTBUFSIZE,
                        input, event);
    
    // Set Server-IP
    uip_ip6addr(&ipaddr, 0xa, 1, 0, 0, 0x206, 0x98ff, 0xfe00, 0x201);
    tcp_socket_connect(&socket, &ipaddr, SERVER_PORT);
    tcp_socket_listen(&socket, 15001);
    printf("socket listening on %d\n", 15001);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tcp_server_process, ev, data)
{
    static struct etimer et;
    static int i=0;
    PROCESS_BEGIN();
    
    start_listening_socket();
   
    etimer_set(&et, SEND_INTERVAL);
    
    while(1) {
    PROCESS_PAUSE();
      if(etimer_expired(&et)) {
          printf("%d Characters sent! Nr. %d \n", tcp_socket_send_str(&socket, "br:Broadcast from Client"), i);
          i++;
          etimer_restart(&et);
      }
    }
    
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/