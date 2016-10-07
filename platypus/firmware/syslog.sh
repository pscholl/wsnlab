#!/bin/sh

dir=/etc/platypus

if [ ! -e $dir/01 -a ! -e $dir/02 -a ! -e $dir/03 ]
then
  rot=01

elif [ -e $dir/01 ]
then
  rm -f $dir/01
  rot=02

elif [ -e $dir/02 ]
then
  rm -f $dir/02
  rot=03

elif [ -e $dir/03 ]
then
  rm -f $dir/03
  rot=01

fi

touch $dir/$rot

while true; do
  journalctl > $dir/syslog$rot.log
  echo "journal written to $dir/syslog$rot.log"
  sleep 2m
done
