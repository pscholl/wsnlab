#!/bin/sh

#exporting TRI_STATE_ALL
if [ ! -d /sys/class/gpio/gpio214 ]; then
	echo 214 > /sys/class/gpio/export
fi

#SPI_2_CLK as GPIO output (IO13 -> 40)
echo 40 > /sys/class/gpio/export
echo 243 > /sys/class/gpio/export
echo low > /sys/class/gpio/gpio214/direction
echo low > /sys/class/gpio/gpio243/direction
echo mode0 > /sys/kernel/debug/gpio_debug/gpio109/current_pinmux
echo high > /sys/class/gpio/gpio261/direction
echo in > /sys/class/gpio/gpio229/direction

#SPI_2_TXD as GPIO output (IO11 -> 43)
echo 43 > /sys/class/gpio/export
echo 262 > /sys/class/gpio/export
echo 241 > /sys/class/gpio/export
echo 259 > /sys/class/gpio/export
echo 227 > /sys/class/gpio/export
echo high > /sys/class/gpio/gpio262/direction
echo low > /sys/class/gpio/gpio241/direction
echo mode0 > /sys/kernel/debug/gpio_debug/gpio115/current_pinmux
echo high > /sys/class/gpio/gpio259/direction
echo in > /sys/class/gpio/gpio227/direction

echo high > /sys/class/gpio/gpio214/direction
