CC=arm-linux-gnueabihf-gcc
AR=arm-linux-gnueabihf-ar


CFLAGS= -I ./include 
LIBS=-L.  -lasound -ldl -l pthread -lm

all:lib_driver.a test_demo

test_demo:main.o driver.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

lib_driver.a:driver.o
	$(AR) -rcs libname.a $^

%o:%c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm *.o
