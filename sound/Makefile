CC=arm-linux-gnueabihf-gcc
AR=arm-linux-gnueabihf-ar


CFLAGS= -I ./include 
LIBS=-L. -lasound -ldl -lpthread -lm

#all:lib_driver.a test_demo
all:sound_test

sound_test:sound_test.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

#lib_driver.a:driver.o
#	$(AR) -rcs libname.a $^

%o:%c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm *.o
