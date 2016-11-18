#!/bin/sh
tar -vczf platypus.tgz \
	platypus/firmware/GrLib/grlib/*.h \
	platypus/firmware/LcdDriver/*.h \
	platypus/firmware/include/ \
	platypus/firmware/lib/ \
	platypus/firmware/bin/ \
	platypus/firmware/*.conf	
