CC=arm-linux-gnueabihf-gcc
AR=arm-linux-gnueabihf-ar


CFLAGS= -I ./include 
LIBS=-L. -ldl -lpthread -lm

all:video_test video_test1

video_test:video_test.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

video_test1:video_test1.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
#lib_driver.a:driver.o
#	$(AR) -rcs libname.a $^

%o:%c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm *.o
