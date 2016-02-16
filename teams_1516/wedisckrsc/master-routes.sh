ifconfig wlan0 add bbbb::7a4b:87ff:fe9f:de59

# 01
route -6 add aa01::206:98ff:fe00:232 gw fe80::7a4b:87ff:fea0:b917 dev wlan0
route -6 add bbbb::7a4b:87ff:fea0:b917 dev wlan0

# 02
route -6 add aa02::206:98ff:fe00:232 gw fe80::7a4b:87ff:feac:e5ea dev wlan0
route -6 add bbbb::7a4b:87ff:feac:e5ea dev wlan0

# 03
route -6 add aa03::206:98ff:fe00:232 gw fe80::fec2:deff:fe3b:683b dev wlan0
route -6 add bbbb::fec2:deff:fe3b:683b dev wlan0

