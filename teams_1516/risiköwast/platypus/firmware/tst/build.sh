#!/bin/sh
gcc -lmraa -Wall -o BatteryGauge MAX17043.c
gcc -lmraa -Wall -o SHARPdisplay SHARPdisplay.c