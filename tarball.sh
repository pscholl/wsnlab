#!/bin/sh
tar -vczf platypus.tgz \
	platypus/firmware/GrLib/grlib/*.h \
	platypus/firmware/LcdDriver/*.h \
	platypus/firmware/include/*.h \
	platypus/firmware/build/*.a \
	platypus/firmware/bin/platypusTest \
	platypus/firmware/platypus.conf	
