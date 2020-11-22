CC=arm-linux-gnueabihf-gcc
AR=arm-linux-gnueabihf-ar


CFLAGS= -I ./include -static
LIBS=-L. -ldl -lpthread -lm

all:video_test video_test1 

video_test:video_test.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	cp $@ ~/nfs

video_test1:video_test1.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	cp $@ ~/nfs
	cp $@ ~/rootfs-tf/usr/bin/


#lib_driver.a:driver.o
#	$(AR) -rcs libname.a $^

%o:%c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm *.o
