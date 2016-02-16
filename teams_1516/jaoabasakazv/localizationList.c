/*
  Creation of an ArrayList and its functions.
  It will store the list of IP addresses of the nodes

*/



#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "udpClient6.h"
#include "localizationList.h"


void list_init(struct arraylist *list) {
  list -> size = 0;
  list -> data = NULL;
}

void list_setData(struct arraylist *list, data_type* data) {
  list-> data = data;
}

int list_get_size(const struct arraylist list) {
  return list.size;
}

void list_add(struct arraylist *list, data_type data) {
 int size = list_get_size(*list);
  data_type *newData;
  newData = realloc(list -> data, (size + 1) * sizeof newData[0]);
  if (newData)
  {
      newData[size] = data;
      list_setData(list, newData);
      ++list -> size;
  }
}

data_type list_get(const struct arraylist list, int index) {
  if(index < list_get_size(list) && index >= 0) {
		return list.data[index];
  }
  else {
		return NULL;
  }
}

int list_indexof(const struct arraylist list, data_type data) {
  int index = 0;
  for(index = 0; index < (list_get_size(list)); index++) {
    if(strcmp(list.data[index], data) == 0) {
		return index;
    }
  }
  return -1;
}

// Adds string address to list if not present already
void addToListWithCheck (struct arraylist *list, data_type value) {
  int index = list_indexof(*list, value);
  if (index != -1) {
    printf("String not added [already present], index of string in arrayList = %d. \n", index);
  } else {
    list_add (list, value);
  }
}

// Gives IP address of left (index - 1) neighbour (wrap around for index = 0) 
char* getLeftOf (struct arraylist list, data_type value) {
  char * leftIp = NULL;
  int index = list_indexof(list, value);
  if (index != -1) {
    if (index == 0) {
      leftIp = list_get(list, (list_get_size (list) - 1));
    }
    else if (index > 0) {
      leftIp = list_get(list, (index-1));
    }
  }
  return leftIp;
}

// Gives IP address of right neighbour (index + 1) (wrap around for last IP in the list) 
char* getRightOf (struct arraylist list, data_type value) {
  char * rightIp = NULL;
  int index = list_indexof(list, value);
  int size = list_get_size(list);
  if (index != -1) {
    if (index == (size - 1)) {
      rightIp = list_get(list, 0);
    }
    else if (index >= 0) {
      rightIp = list_get(list, (index+1));
    }
  }
  return rightIp;
}
