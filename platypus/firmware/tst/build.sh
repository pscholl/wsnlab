#!/bin/sh
gcc -lmraa -Wall -o BatteryGauge MAX17043.c
gcc -lmraa -Wall -o SHARPdisplay SHARPdisplay.c


gcc -Wall -I../GrLib/grlib -I../LcdDriver -lmraa -c dsptest.c
gcc -Wall -I../GrLib/grlib -I../LcdDriver -lmraa -o dsptest dsptest.o ../LcdDriver/LcdDriver.c ../LcdDriver/Sharp96x96.c ../GrLib/fonts/fontfixed6x8.c ../GrLib/grlib/context.c ../GrLib/grlib/display.c ../GrLib/grlib/line.c ../GrLib/grlib/circle.c ../GrLib/grlib/string.c

gcc -Wall -lmraa imutest.c -o imutest
