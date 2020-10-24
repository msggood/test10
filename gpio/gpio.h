#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

#include <linux/types.h>

/* IOCTL commands */

#define GPIO_CONTROL_IOC_MAGIC			'l'

#define GPIO_CONTROL_REQ		_IOW(GPIO_CONTROL_IOC_MAGIC, 0, __u32)
#define GPIO_CONTROL_OUTPUT	    _IOW(GPIO_CONTROL_IOC_MAGIC, 1, __u32)
#define GPIO_CONTROL_INTPUT	    _IOW(GPIO_CONTROL_IOC_MAGIC, 2, __u32)
#define GPIO_CONTROL_GETVAUE  	_IOW(GPIO_CONTROL_IOC_MAGIC, 3, __u32)
#define GPIO_CONTROL_FREE     	_IOW(GPIO_CONTROL_IOC_MAGIC, 4, __u32)

struct gpio_data
{
    int pin;
    int value;
};

#endif 

