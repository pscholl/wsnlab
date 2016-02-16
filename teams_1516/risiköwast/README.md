Wireless Sensor Networks
========================
University of Freiburg - Winter Term 2015/16
--------------------------------------------
In the following we shortly describe how to use our source code. Further information about the setup and structure of this project can be found in our short presentation: “WSN project documentation.pdf“. 



Contiki TCP Server / Client:
============================

Init\_script\_client.sh: This is a simple bash-script to setup IPv6 addresses and routes of a contiki client node. To start a certain contiki-programm the path has to be adapted.

Init\_script\_server.sh: Same as for client, but in addition all client routes are set manually.

Makefile: Standard Makefile with some additions to configure contiki correctly and use ipv6.

Makefile.target: Defines minimal-net as default platform for contiki.

tcp-client.c: Simple contiki ipv6 client which connects to tcp-server on predefined port and ip-address. Broadcasts messages in a certain period.

    make
    ./Init_script_client.sh

tcp-server.c: Simple contiki ipv6 server, listens on ports 15000-15004. Replies to incoming messages.

    make
    ./Init_script_server.sh

IPv6 TCP Server / Client:
=========================
Manual implementation of multiclient ipv6 sockets server/client with pthreads in order to supplement contiki usage. Both user and client utilise socketlayer.c and socketprx.h from the book "C from A is Z". Default port for connection is 15000. Accepts connection from "any".

Usage:
------
```
make
./server
./client a::IP
```

```
./server
./client a::IP
```

Once users are connected, broadcasts are sent with br: whereas unicasts to specific users are sent with uX: as follows.

```
br:Text
u0:Text
```

Platypus
========
Platypus implements the usage of IMU with interrupts. Initialization of Stickman by multiple IMU interrupts (taps). The function pollIMU checks for "faceDown", multiple IMU interrupts on different Platypus boards cause Stickman to travel between screens. All display functions can be seen in "animation.cpp" and "animation.h" located in "wsnlab-master/platypus/firmware/src" and "wsnlab-master/platypus/firmware/include". The migration of sockets_ipv6 files "socketlayer.c" and "socketprx.h" are located in the same folders.

Usage:
------
Make sure you have an IPv6 address. If not add it by using ip addr -6 add.
```
ip -6 addr add a::IP/64 dev wlan0
```
Start the ipv6 server as mentioned above. Important: The IPv6 address of the server is hard-coded into platypus.cpp!!! Then execute the following in "wsnlab-master/platypus/firmware".
```
make
/b.in/platypusMain
```
