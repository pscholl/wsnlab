# WSN_Lab-Contiki-


Configuration Process
1. Enable the Access Point Mode in the node which is going to act as the central server of the network:
configure_edison --enableOneTimeSetup

2. Start the advertisement of the IPv6 prefix in the central node:
radvd -C /etc/radvd.conf

3. There will be 3 sensor nodes in the network. In each of them it is needed first to connect to the network hosted by the central node (192.168.42.xx) and also enable the IPv6 address using the following command:
ip addr add fe80:: dev wlan0

4. Run the sensor firmware in every sensor node. It is located in the folder /wsnlab/platypus/firmware/bin/SensorNodeMain.o

5. Run the display firmware in every sensor node. It is located in the folder /wsnlab/platypus/firmware/bin/udp_ipv6_displayMain.o

6. Finally run the central node firmware in the central server. Its location is /central_Node/central.o

7. The localization process is based on accelerometer events. To generate one alarm of this type, one needs to move the node. This process should be executed first, to set an order between the sensor nodes. The node which is moved first, will be located in the first position of the list, and so on. Once all the sensors are in the list (it depends of the total number of sensors, which is defined in the central node firmware), the localization process finishes and we can proceed with the normal functions of the network. The figure will be displayed in the leftmost sensor.

8. If the temperature increases above a pre-defined threshold, the figure will move to the furthest node from the one that sent the temperature warning.

9. If the node is moved and the figure is present there, it will fall down.

10. Whenever someone blows (the humidity should go over a threshold) in a sensor node, the figure will be transported to that specific node.
