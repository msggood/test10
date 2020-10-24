#!/bin/sh
#./configure --prefix=./_install --enable-static --disable-x86asm
#./configure --host=arm-linux-gnueabihf CC=/home/jack/rk/buildroot/output/rockchip_rv1126_rv1109/host/bin/arm-linux-gnueabihf-gcc --prefix=`pwd`/_install
./configure --cross-prefix=arm-linux-gnueabihf- --enable-cross-compile --target-os=linux --cc=arm-linux-gnueabihf-gcc --arch=arm --prefix=`pwd`/_install --enable-static --enable-gpl 

