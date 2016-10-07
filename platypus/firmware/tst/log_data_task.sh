#!/bin/bash
date=$(date +"%b %d %T")
echo -n "$date: " >> /home/log.txt

/home/platypus/firmware/tst/BatteryGauge >> /home/log.txt

