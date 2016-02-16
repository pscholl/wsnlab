#!/bin/bash

echo checking opkg base-feeds.conf
if grep -q "opkg" /etc/opkg/base-feeds.conf
then
	echo no update required
else
	echo adding opkg.net
	echo "src/gz edison http://repo.opkg.net/edison/repo/edison" >> /etc/opkg/base-feeds.conf
	echo "src/gz core2-32 http://repo.opkg.net/edison/repo/core2-32" >> /etc/opkg/base-feeds.conf
fi

echo installing cronie
opkg update
opkg install cronie
chmod a+x /home/platypus/firmware/tst/log_data_task.sh

echo building tests
gcc -lmraa -Wall -o BatteryGauge MAX17043.c

echo updating /etc/crontab
if crontab -l | grep -q "log_data_task.sh"
then
	echo task already there, no update required
else
	echo adding cron task
	(crontab -l; echo "0,10,20,30,40,50 * * * * /home/platypus/firmware/tst/log_data_task.sh")|crontab -
fi
crontab -l
