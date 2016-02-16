/* socketprx.h für Linux/UNIX */
#ifndef SOCKETPRX_H_
#define SOCKETPRX_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

/* ein eigener primitver Datentyp für den Socket-Deskriptor */ #define socket_t int
/* Funktionsprototypen */
void error_exit(char *error_message);
int create_socket( int af, int type, int protocol );
void bind_socket(socket_t *sock, struct in6_addr adress, unsigned short port);
void listen_socket( socket_t *sock );
void accept_socket( socket_t *new_socket, socket_t *socket );
void connect_socket(socket_t *sock, char *serv_addr, unsigned short port);
void TCP_send( socket_t *sock, char *data, size_t size);
void TCP_recv( socket_t *sock, char *data, size_t size);
void UDP_send ( socket_t *sock, char *data, size_t size, char *addr, unsigned short port);
void UDP_recv( socket_t *sock, char *data, size_t size);
void close_socket( socket_t *sock );
void cleanup(void);
#endif