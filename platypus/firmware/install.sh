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
echo "> Copying some relevant files"
cp platypus.service /lib/systemd/system/
cp bin/platypusMain /home/root/
cp config.cfg /home/root/

echo
echo "> Enabling platypus autostart service"
systemctl daemon-reload
systemctl enable platypus

echo
while true; do
  read -p "> Install complete. Reboot? " yn
  case $yn in
    [Yy]* ) reboot; break;;
    [Nn]* ) systemctl start platypus; exit;;
    * ) echo "> Please answer yes or no.";;
  esac
done

