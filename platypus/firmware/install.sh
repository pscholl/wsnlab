#!/bin/bash

echo
echo "> please make sure this script is run in 'platypus/firmware/'"

echo
echo "> updating time zone to CET"
rm /etc/localtime
ln -s /usr/share/zoneinfo/Europe/Paris /etc/localtime
date

echo
echo "> Making sure some required software is up-to-date. This may take a while."
echo "> If these are not available you need to update your /etc/opkg/base-feeds.conf! (see https://github.com/pscholl/wsnlab/wiki/edison#3-package-control)"
opkg update
opkg install make cmake coreutils packagegroup-core-buildessential
opkg install mraa
opkg install boost-dev
#opkg install libquadmath-dev

echo
echo "> Building firmware"
mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j && cd ..

echo
echo "> Testing platypus"
bin/platypusTest --id-only --config platypus.conf

echo
echo "> Copying some relevant files"
mkdir -p /etc/platypus
cp bin/platypusTest /usr/bin/platypus-test

cp platypus-test.service /lib/systemd/system/

cp platypus.conf /etc/platypus/

systemctl daemon-reload

echo "> Install complete."
#echo
#while true; do
#  read -p "> Install complete. Reboot? " yn
#  case $yn in
#    [Yy]* ) reboot; break;;
#    [Nn]* ) exit;;
#    * ) echo "> Please answer yes or no.";;
#  esac
#done

