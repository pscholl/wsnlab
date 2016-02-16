#!/usr/bin/env python
import sys
import subprocess

#Get MAC-address and convert it to IPV6 address in bbbb-subnet
ARP=subprocess.check_output('ifconfig wlan0',shell=True)
MAC_hex=ARP[(37):(37+18)]
m = hex(int(MAC_hex.translate(None,' .:-'),16)^0x020000000000)[2:]
MAC='bbbb::%s:%sff:fe%s:%s' %(m[:4],m[4:6],m[6:8],m[8:12])
# Add bbbb-prefix for wlan0
subprocess.check_output("ifconfig wlan0 add "+MAC+"/64",shell=True)
# Delete wrong route
subprocess.check_output("ip route del bbbb::/64 dev wlan0",shell=True)
# Default route to Access-point
subprocess.check_output("route -6 add ::1/0 gw fe80::7a4b:87ff:fe9f:de59 dev wlan0",shell=True)

