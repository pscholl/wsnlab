#!/bin/sh
tar -vczf platypus-src.tgz \
	platypus/firmware/LcdDriver/ \
	platypus/firmware/include/ \
	platypus/firmware/tst/ \
	platypus/firmware/*.conf \
	platypus/firmware/GrLib/ \
	platypus/firmware/CMake* \
	platypus/firmware/src/
