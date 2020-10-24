#include <linux/init.h>
#include <linux/types.h>
#include <linux/input/mt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/major.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/rcupdate.h>
#include <linux/cdev.h>
#include <linux/gpio.h>

#include "gpio.h"


static struct class *gpio_class;
#if 0
struct gpio_gpio
{
    int red_led;
    int green_led;
    int buzzer;
    int motor;
};

static struct gpio_gpio gpio = {
    .red_led = GPIO_PC(17),
    .green_led = GPIO_PB(29),
    .buzzer = GPIO_PB(31),
    .motor = GPIO_PA(25),
};
#endif
#define MAX_COUNT  20
static int gpios[MAX_COUNT];
static int current_index=0;
static int open(struct inode *inode,struct file *file)
{
    return 0;    
}

//gpio_direction_output(gpio.red_led, state);

//int gpio_set_cfgpin(int pin, int val);
//int gpio_input(int pin);
//int gpio_output(int pin, int val);

int gpio_request(unsigned gpio, const char *label);
int gpio_direction_input(unsigned gpio);
int gpio_direction_output(unsigned gpio, int value);
int gpio_get_value(unsigned gpio);


static long ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{    
    int ret;
    int i;
    int gpio_num;
    struct gpio_data *data=(gpio_data*)arg;
    char name[128];
    memset(name,"\0",128);
    sprintf(name,"gpio_%d",data->pin);
    switch(cmd)
    {
        case GPIO_CONTROL_REQ:            
            ret = gpio_request(data->pin,name);       
            if(ret){                                                         
                  printk("gpio %d requrest fail\n",data->pin);
                  return -1;
            }   
            gpios[current_index]=data->pin;
            current_index++;
            break;
            
        case GPIO_CONTROL_OUTPUT:            
            ret=gpio_direction_output(data->pin,data->value);
            break;
        case GPIO_CONTROL_INTPUT:
            gpio_direction_input(data->pin);
//            ret=gpio_get_value(data->pin);            
            break;       
              
        case GPIO_CONTROL_GETVAUE:   
//            gpio_direction_input(data->pin);
            ret=gpio_get_value(data->pin);            
            break;       
        
        case GPIO_CONTROL_FREE:   
        
            for(i=0;i<MAX_COUNT;i++)
            {
                if(gpios[i]>0)
                    gpio_free(gpios[i]);
                gpios[i]=-1;
            }   
            break;       
        
        

       
        default:
            printk("cmd:%x,arg=%ld",cmd,arg);        
    }
    
    return ret;
}

static const struct file_operations gpio_fops =
{                   
    .owner = THIS_MODULE,
    .open  = open,
    .unlocked_ioctl = ioctl,
};
static struct cdev *gpio_cdev;
struct device *class_dev;
#if 0
void init_gpio(void)
{
    int ret;
    ret = gpio_request(gpio.red_led,"leds buzzer red_led");       
    if(ret){                                                         
          printk("gpio %d requrest fail\n",gpio.red_led);
    }                                                                 

    ret = gpio_request(gpio.green_led,"leds buzzer green_led");       
    if(ret){                                                         
          printk("gpio %d requrest fail\n",gpio.green_led);
    }       
    
    ret = gpio_request(gpio.buzzer,"leds buzzer buzzer");       
    if(ret){                                                         
          printk("gpio %d requrest fail\n",gpio.buzzer);
    }       
    ret = gpio_request(gpio.motor,"leds buzzer buzzer");       
    if(ret){                                                         
          printk("gpio %d requrest fail\n",gpio.motor);
    }           
}
#endif
static int __init gpio_init(void)
{
    int i=0;

    dev_t gpio_devid;
//    init_gpio();
    alloc_chrdev_region(&gpio_devid, 0, 32768, "gpio_control");
#if 1
    gpio_cdev = cdev_alloc();
    cdev_init(gpio_cdev, &gpio_fops);
    gpio_cdev->owner = THIS_MODULE;
    cdev_add(gpio_cdev, gpio_devid, 1);  //
#else    
    register_chrdev(231, "gpio", &gpio_fops);
#endif
    gpio_class=class_create(THIS_MODULE, "gpio_control");
    class_dev=device_create(gpio_class, NULL, gpio_devid, NULL, "gpio_control");
    
    for(i=0;i<MAX_COUNT;i++)
        gpios[i]=-1;

    return 0;
}
static void __exit gpio_exit(void)
{
    int i;
//    printk("gpio_exit\n"); 

    for(i=0;i<MAX_COUNT;i++)
    {
        if(gpios[i]>0)
            gpio_free(gpios[i]);
        gpios[i]=-1;
    }   
//    gpio_free(gpio.red_led);   
#if 1    
    cdev_del(gpio_cdev);    
#else
//    unregister_chrdev(231, "gpio");
#endif
    device_unregister(class_dev);
    class_destroy(gpio_class);
}
module_init(gpio_init);
module_exit(gpio_exit);
MODULE_LICENSE("GPL");
