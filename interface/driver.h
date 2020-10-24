
/*VIDO */

int video_driver_init();

void video_driver_get_resolution(int * width, int * height);

/*
Mode 按位定义:
Bit 0-7 位 0xFF=忽略, 有效值为 0-3,设置屏幕输出逆时针旋转 0,90,180,270 度。
Bit 8-15 位 0xFF=忽略,
Bit 8 位 缩放方式: 0=等比例拉伸, 1 =满屏显示。
Bit 9 位 缩放消锯齿: 0=关闭 1=如果硬件支持的话打开硬件双线性渲染
Bit 10 位 垂直同步: 0=关闭 1=强制垂直同步
*/

int video_driver_set_mode(int mode);

void video_driver_disp_frame(void * data, int img_width, int img_height, size_t pitch);

int video_driver_deinit();


/*sound */

int sound_driver_init(int outdev, int samples, int channel);

int sound_driver_playframe(void * data, int frames);

int sound_driver_deinit();

/*gpio */
int gpio_init();

int gpio_set_cfgpin(int pin, int val);

int gpio_input(int pin);

int gpio_output(int pin, int val);

int gpio_deinit();

//drivers/mtd/devices/m25p80.c
//drivers/spi/spi.c
//03h(读),06h(写使能),20h(擦除),02h



