CC=arm-linux-gnueabihf-gcc
AR=arm-linux-gnueabihf-ar


CFLAGS= -I ./include -static
LIBS=-L. -lasound -ldl -lpthread

#all:lib_driver.a test_demo
all:sound_test

sound_test:sound_test.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	cp $@ ~/nfs

#lib_driver.a:driver.o
#	$(AR) -rcs libname.a $^

%o:%c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm *.o
