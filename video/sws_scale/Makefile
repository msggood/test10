#CC=gcc
CC=arm-linux-gnueabihf-gcc
#SRCS=$(wildcard *.c */*.c)
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))
CFLAG=-g -I./include
LIB=-L./lib -lswscale -lavutil -lm
NAME=$(wildcard *.c)
TARGET=scale_video_test

$(TARGET):$(OBJS)
	$(CC) $(CFLAG) -o $@ $^ $(LIB)

%.o:%.c
	$(CC) $(CFLAG) -o $@ -c $< -g

clean:
	rm *.o $(TARGET)


