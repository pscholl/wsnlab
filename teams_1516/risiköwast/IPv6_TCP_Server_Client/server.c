/* multi_server.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "socketprx.h"

#include <termios.h>
#include <unistd.h>

#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <time.h>

// Exit nicely
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
// / Exit nicely

#define BUF 1024
char n[1024];

socket_t sock1, sock2, sock3;
fd_set gesamt_sock, lese_sock;
int sock_max, i, max=-1;
int client_sock[FD_SETSIZE];

// Assumes 0 <= max <= RAND_MAX
// Returns in the half-open interval [0, max]
long random_at_most(long max) {
  unsigned long
  // max <= RAND_MAX < ULONG_MAX, so this is okay.
  num_bins = (unsigned long) max + 1,
  num_rand = (unsigned long) RAND_MAX + 1,
  bin_size = num_rand / num_bins,
  defect   = num_rand % num_bins;

  long x;
  do {
    x = random();
  } while (num_rand - defect <= (unsigned long)x);

  // Truncated division is intentional
  return x/bin_size;
}

void broadcastMessage(char* buffer)
{
  puts(buffer);

  for(i=0; i<=max; i++) {
    if((sock3 = client_sock[i]) < 0)
      continue;
    TCP_send(&sock3, buffer, strlen (buffer));
  }
}

void unicastTo(char* buffer, int i)
{
  puts(buffer);

  if((sock3 = client_sock[i]) >= 0)
    TCP_send(&sock3, buffer, strlen (buffer));
}

// Exit nicely
void my_handler(int s){
  broadcastMessage("quit\n");
  for(i=0; i<=max; i++)
    close_socket(&client_sock[i]);
  exit(0);
}
// / Exit nicely

void *broadcaster()
{
  char *buffer = (char *)malloc (BUF);

  do {
    fgets (buffer, BUF, stdin);

    char delimiter[] = ":";
    char *ptr = strtok(buffer, delimiter);

    if (strcmp(ptr,"br") == 0) {
      ptr = strtok(NULL, delimiter);
      broadcastMessage(ptr);
    } else if (ptr[0] == 'u') {
      ptr = ptr + 1;
      int destClient = atoi(ptr);
      ptr = strtok(NULL, delimiter);
      if(ptr != NULL) {
        unicastTo(ptr, destClient);
        if (strcmp (ptr, "quit\n") == 0) {
          close_socket (&sock3);        //Socket schließen
        }
      }
    }
  } while (strcmp (buffer, "quit\n") != 0);
  close_socket (&sock3);
  exit(0);
}

char* parseBuffer(char *buffer) {
  printf("\nbuffer: %c\n", buffer[0]);
  return buffer;
}

void *listener()
{
  int i, ready;
  char *buffer = (char*) malloc (BUF);

  for (;;) {
    // Immer aktualisieren
    lese_sock = gesamt_sock;

    /* Hier wird auf die Ankunft von Daten oder
     * neuer Verbindungen von Clients gewartet. */
    ready = select( sock_max+1, &lese_sock, NULL, NULL, NULL );

    // Eine neue Client-Verbindung ...?
    if( FD_ISSET(sock1, &lese_sock)) {
      accept_socket( &sock1, &sock2 );

      /* Freien Platz für (Socket-)Deskriptor
       * in client_sock suchen und vergeben */
      for(i = 0; i < FD_SETSIZE; i++)
        if(client_sock[i] < 0) {
          client_sock[i] = sock2;
          break;
        }
      /* mehr als FD_SETSIZE Clients sind nicht möglich */
      if( i == FD_SETSIZE )
        error_exit("\n[-] Server überlastet - zu viele Clients\n");
      /* den neuen (Socket-)Deskriptor zur
       * (Gesamt)Menge hinzufügen */

      FD_SET(sock2, &gesamt_sock);

      /* select() benötigt die höchste
       * (Socket-)Deskriptor-Nummer. */
      if( sock2 > sock_max ){
        sock_max = sock2;
      }
      /* höchster Index für client_sock
       * für die anschließende Schleife benötigt */
      if( i > max )
        max = i;

      // Assign user ID:
      sprintf(buffer, "0,0,%d", i);
      broadcastMessage(buffer);

      /* ... weitere (Lese-)Deskriptoren bereit? */
      if( --ready <= 0 )
        continue; //Nein ...

    } //if(FD_ISSET ...


    /* Ab hier werden alle Verbindungen von Clients auf
     * die Ankunft von neuen Daten überprüft. */
    for(i=0; i<=max; i++) {

      if((sock3 = client_sock[i]) < 0)
        continue;
      /* (Socket-)Deskriptor gesetzt ... */
      if(FD_ISSET(sock3, &lese_sock)){
        /* ... dann die Daten lesen */
        TCP_recv (&sock3, buffer, BUF-1);

//        buffer = parseBuffer(buffer);

        char delimiter[] = ":";
        char *ptr_br = "br";
        const char *ptr_u = "u";
        char *ptr;
        // initialisieren und ersten Abschnitt erstellen
        ptr = strtok(buffer, delimiter);

		// We got a broadcast
        if (strcmp(ptr,ptr_br) == 0) {
          ptr = strtok(NULL, delimiter);
          if(ptr != NULL) {
            int command = atoi(ptr);

            switch (command) {
              case 4: {  // We are playing a game: the only game -> hideandseek
                if (max >= 0) {
                  // Generate random number
                  int chosenClient = -1;
                  chosenClient = (int)random_at_most((long)max);
                  printf("Chosen: %d\n",chosenClient);
                  char *message = (char *)malloc (BUF);
                  sprintf(message, "1,%d,4,%d", i, chosenClient);
                  broadcastMessage(message);
                } else {
                  printf("[-] 9461: Not enough clients.\n");
                }
                break;
              }

              default: {
                char *message = (char *)malloc (BUF);
                sprintf(message, "1,%d,%s", i, ptr);
                broadcastMessage(message);
                break;
              }
            }
//            ptr = strtok(NULL, delimiter);
          }

		// We got a unicast
        } else if (ptr[0] == ptr_u[0]) {  // u33425: test
          ptr = ptr + 1;
          int destClient = atoi(ptr);
          ptr = strtok(NULL, delimiter);
          if(ptr != NULL) {
            char *message = (char *)malloc (BUF);
            sprintf(message, "1,%d,%s", i, ptr);
            unicastTo(message,destClient);
          }
        } else if (strcmp (buffer, "quit\n") == 0) {
          /* ... hat sich der Client beendet. */
          close_socket (&sock3);        //Socket schließen
          FD_CLR(sock3, &gesamt_sock);  //aus Menge löschen
          client_sock[i] = -1;          //auf -1 setzen
          sprintf(buffer, "\n[~] Client %d hat sich beendet.\n", i);
          broadcastMessage(buffer);
        }  else {
          printf("[%d] %s\n", i, buffer);
        }

        /* Sind noch lesbare Deskriptoren vorhanden ...? */
        if( --ready <= 0 )
          break; //Nein ...
      }
    }
  } // for(;;)
}// *listener()

int main (void) {
  // Exit nicely
  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);
  // / Exit nicely


  sock_max = sock1 = create_socket(AF_INET6, SOCK_STREAM, 0);
  atexit(cleanup);
  bind_socket( &sock1, in6addr_any, 15000 );
  listen_socket (&sock1);

  for(i = 0; i < FD_SETSIZE; i++)
    client_sock[i] = -1;
  FD_ZERO(&gesamt_sock);
  FD_SET(sock1, &gesamt_sock);

  pthread_t tr;
  pthread_create(&tr,NULL,listener,NULL);

  pthread_t tb;
  pthread_create(&tb,NULL,broadcaster,NULL);

  pthread_join(tr,NULL);
  pthread_join(tb,NULL);

  return EXIT_SUCCESS;
}
