/* client.c */
#include <stdbool.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "socketprx.h"

#include <termios.h>
#include <unistd.h>

#include <pthread.h>

#define BUF 1024

// Exit nicely
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
// / Exit nicely

socket_t sock;
bool broadcast = false;

// initialisieren und ersten Abschnitt erstellen
char delimiter[] = ":";
char *ptr_cmp = "br";
char *ptr;

// Exit nicely
void my_handler(int s) {
  TCP_send (&sock, "quit\n", strlen ("quit\n"));
  close_socket (&sock);
  exit(0);
}
// / Exit nicely

void *listener()
{
  char *buffer = (char *)malloc (BUF);
  do {
    buffer[0] = '\0';
    TCP_recv (&sock, buffer, BUF-1);
    if(strcmp (buffer, "quit\n") != 0)
        printf ("\nNachricht erhalten: %s\n", buffer);

        // initialisieren und ersten Abschnitt erstellen
        ptr = strtok(buffer, delimiter);

        if (strcmp(ptr,ptr_cmp) == 0) {
          ptr = strtok(NULL, delimiter);
          while(ptr != NULL) {
            printf("Client Befehl: %s\n", ptr);
            ptr = strtok(NULL, delimiter);
          }
        }
  } while (strcmp (buffer, "quit\n") != 0);
  close_socket (&sock);
  exit(0);
}

void *sender()
{
  char *buffer = (char *)malloc (BUF);

  do {
    fgets (buffer, BUF, stdin);
    TCP_send (&sock, buffer, strlen (buffer));
  } while (strcmp (buffer, "quit\n") != 0);
  close_socket (&sock);
  exit(0);
}

int main (int argc, char *argv[]) {

  // Exit nicely
  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);
  // / Exit nicely

  if( argc < 2 ){
    printf("Usage: %s ServerAdresse\n", *argv);
    exit(EXIT_FAILURE);
  }

  sock = create_socket(AF_INET6, SOCK_STREAM, 0);
  atexit(cleanup);
  connect_socket(&sock, argv[1], 15000);

  pthread_t tw;
  pthread_create(&tw,NULL,sender,NULL);

  pthread_t tr;
  pthread_create(&tr,NULL,listener,NULL);

  pthread_join(tw,NULL);
  pthread_join(tr,NULL);

  return EXIT_SUCCESS;
}
