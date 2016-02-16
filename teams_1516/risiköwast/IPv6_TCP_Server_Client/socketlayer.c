/* socketlayer.c - für Linux/UNIX */
#include "socketprx.h"
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

/*
 * Daten senden via UDP *
void UDP_send ( socket_t *sock, char *data, size_t size, char *addr, unsigned short port){
  struct sockaddr_in6 addr_sento;
  struct hostent *h;
  int rc;
  * IP-Adresse des Servers überprüfen *
  h = gethostbyname(addr);
  if (h == NULL)
    error_exit("Unbekannter Host?");
  addr_sento.sin_family = h->h_addrtype;
  memcpy ( (char *) &addr_sento.sin_addr.s_addr,
          h->h_addr_list[0], h->h_length); addr_sento.sin_port = htons (port);
  rc = sendto(*sock, data, size, 0,
              (struct sockaddr *) &addr_sento,
              sizeof (addr_sento));
  if (rc < 0)
    error_exit("Konnte Daten nicht senden - sendto()");
}
* Daten empfangen via UDP *
void UDP_recv( socket_t *sock, char *data, size_t size){
  struct sockaddr_in6 addr_recvfrom;
  unsigned int len;
  int n;
  len = sizeof (addr_recvfrom);
  n = recvfrom ( *sock, data, size, 0,
                (struct sockaddr *) &addr_recvfrom, &len );
  if (n < 0) {
    printf ("Keine Daten empfangen ...\n");
    return;
  }
}
*/