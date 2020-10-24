#!/bin/sh
./configure CC=arm-linux-gnueabihf-gcc --target=arm-linux --host=x86_64 --prefix=`pwd`/_install
