#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>

#include "gpio.h"


#define OK   1
#define FAIL 0

int gpio_fd;


int gpio_init();
int gpio_set_cfgpin(int pin, int val);
int gpio_input(int pin);
int gpio_output(int pin, int val);
int gpio_deinit();


int gpio_init()
{    
    gpio_fd =open("/dev/gpio_control",O_RDWR);
    if(gpio_fd<0)
    {
       printf("open /dev/gpio_control error!\n");
       return FAIL;
    }
    return OK;
}


int gpio_set_cfgpin(int pin, int val)
{    
    int ret;
    struct gpio_data data;
    data.pin=pin;
    ret=ioctl(gpio_fd,GPIO_CONTROL_REQ,&data);
    if(ret<0)
    {
        printf("GPIO_CONTROL_REQ error\n");
        return FAIL;
    }
    if(val==0)
    {
        ret=ioctl(gpio_fd,GPIO_CONTROL_INTPUT,&data);        
        if(ret<0)
        {
            printf("GPIO_CONTROL_INTPUT error\n");
            return FAIL;
        }
    }
    return OK;
}
int gpio_input(int pin)
{
    int ret=0;
    struct gpio_data data;
    data.pin=pin;    
    ret=ioctl(gpio_fd,GPIO_CONTROL_GETVAUE,&data);    
    if(ret<0)
    {
        printf("GPIO_CONTROL_OUTPUT error\n");
        return FAIL;
    }    
    return OK;
}

int gpio_output(int pin, int val)
{   
    int ret=0;
    struct gpio_data data;
    data.pin=pin;
    data.value=val;    
    ret=ioctl(gpio_fd,GPIO_CONTROL_OUTPUT,&data);
    if(ret<0)
    {
        printf("GPIO_CONTROL_OUTPUT error\n");
        return FAIL;
    }    
    return OK;
}

int gpio_deinit()
{
    struct gpio_data data;
    ioctl(gpio_fd,GPIO_CONTROL_FREE,&data);    
    close(gpio_fd);
    return OK;
};

int main()
{
    gpio_init();
    gpio_set_cfgpin(2, 0);
    gpio_input(2);
    gpio_set_cfgpin(3, 1);    
    gpio_output(3, 1);
    gpio_deinit();
    return 0;    
}
