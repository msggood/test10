#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<fcntl.h>
#include<linux/fb.h>

#include <math.h>

#define OK   1
#define FAIL 0

int video_driver_init()
{
    return OK;
}


void video_driver_get_resolution(int * width, int * height)
{
    int fd;
    struct fb_var_screeninfo screen_info;
    fd = open("/dev/fb0",O_RDWR);
    if(fd<0)
    {
        printf("open /dev/fb0 error\n");
    }
    ioctl(fd,FBIOGET_VSCREENINFO,&screen_info);
    printf("%d*%d\n",screen_info.xres,screen_info.yres);
    *width=screen_info.xres;
    *height=screen_info.yres;
    close(fd);
}

/*
Mode 按位定义:
Bit 0-7 位 0xFF=忽略, 有效值为 0-3,设置屏幕输出逆时针旋转 0,90,180,270 度。
Bit 8-15 位 0xFF=忽略,
Bit 8 位 缩放方式: 0=等比例拉伸, 1 =满屏显示。
Bit 9 位 缩放消锯齿: 0=关闭 1=如果硬件支持的话打开硬件双线性渲染
Bit 10 位 垂直同步: 0=关闭 1=强制垂直同步
*/

int video_driver_set_mode(int mode) 
{
    return OK;
}

/*
将源图像数据通过硬件 scaler 适应到当前输出屏幕
Data 源图像数据地址,格式为 RGB565
img_width 显示有效宽度
img_height 显示有效高度
Pitch 源图像行宽,一行的字节数
*/

void video_driver_disp_frame(void * data, int img_width, int img_height, size_t pitch)
{

}

int video_driver_deinit()
{
    return OK;
}

int main()
{
    int w,h;
    video_driver_get_resolution(&w,&h);
    printf("w=%d,h=%d\n",w,h);
    return 0;
}
