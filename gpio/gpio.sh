#!/bin/sh
pin=89 
echo $pin > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio$pin/direction
echo 0 > /sys/class/gpio/gpio$pin/value
