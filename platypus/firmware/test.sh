#!/bin/bash

echo
echo "> updating time zone to CET"
rm /etc/localtime
ln -s /usr/share/zoneinfo/Europe/Paris /etc/localtime
date

echo
echo "> Making sure MRAA is up-to-date"
opkg install libmraa0

echo
echo "> Building firmware"
make -j


echo
echo "> Testing platypus"
bin/ppsTestMain --id-only

