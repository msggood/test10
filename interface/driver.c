#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<fcntl.h>
#include<linux/fb.h>

#include <alsa/asoundlib.h>
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

/*sound */
/*
Outdev 音频输出设备选择(0=耳机 1=外放设备)
Samples 采样率, 目前默认设置为 44100
Channel 单声道(1)或立体声(2), 目前默认设置为 2
*/

static snd_pcm_t *handle;
int sound_driver_init(int outdev, int samples, int channel)
{

    int rc;
    int size;
    
    unsigned int val;

    int dir;
    snd_pcm_uframes_t frames;
    char *buffer;
    char *inFile;

    int fd;
    int err;

    //以播放模式打开设备
    rc = snd_pcm_open(&handle, "default",SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) 
    {
        fprintf(stderr,"unable to open pcm device: %s\n",snd_strerror(rc));
        exit(1);
    }
    //配置硬件参数结构体
    snd_pcm_hw_params_t *params;
    
    //params申请内存
    snd_pcm_hw_params_malloc(&params);

     //使用pcm设备初始化hwparams
    err=snd_pcm_hw_params_any(handle, params);
    if (err < 0)
    {
        fprintf(stderr, "Can not configure this PCM device: %s\n",snd_strerror(err));
        exit(1);
    }
    
    //设置多路数据在buffer中的存储方式
    //SND_PCM_ACCESS_RW_INTERLEAVED每个周期(period)左右声道的数据交叉存放
    err=snd_pcm_hw_params_set_access(handle, params,SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) 
    {
        fprintf(stderr,"Failed to set PCM device to interleaved: %s\n",snd_strerror(err));
        exit(1);
    }
    
    //设置16位采样格式，S16为有符号16位,LE是小端模式
    err=snd_pcm_hw_params_set_format(handle, params,SND_PCM_FORMAT_S16_LE);
    if (err < 0)
    {
        fprintf(stderr,"Failed to set PCM device to 16-bit signed PCM: %s\n",snd_strerror(err));
        exit(1);
    }
    
    //设置声道数,双声道
    err=snd_pcm_hw_params_set_channels(handle, params, channel);
    if (err < 0)
    {
        fprintf(stderr, "Failed to set PCM device to mono: %s\n",snd_strerror(err));
        exit(1);
    }
    
    //采样率48000
    val = samples;
    //设置采样率,如果采样率不支持，会用硬件支持最接近的采样率
    err=snd_pcm_hw_params_set_rate_near(handle, params,&val, &dir);
    if (err < 0) 
    {
        fprintf(stderr, "Failed to set PCM device to sample rate =%d: %s\n",val,snd_strerror(err));
        exit(1);
    }
    
    unsigned int buffer_time,period_time;
    //获取最大的缓冲时间,buffer_time单位为us,500000us=0.5s
    snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, 0);
    if ( buffer_time >500000)
        buffer_time = 500000;
    
    //设置缓冲时间
    err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, 0);
    if (err < 0) 
    {
        fprintf(stderr, "Failed to set PCM device to buffer time =%d: %s\n",buffer_time,snd_strerror(err));
        exit(1);
    }

#if 1 
    period_time = 26315;
    //设置周期时间
    err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, 0);
    if (err < 0) 
    {
        fprintf(stderr, "Failed to set PCM device to period time =%d: %s\n",period_time,snd_strerror(err));
        exit(1);
    }
#endif

    //让这些参数作用于PCM设备
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) 
    {
        fprintf(stderr,"unable to set hw parameters: %s\n",snd_strerror(rc));
        exit(1);
    }

    snd_pcm_hw_params_get_period_size(params, &frames,&dir);
    // 1 frame = channels * sample_size.
    //size = frames * FSIZE; /* 2 bytes/sample, 1 channels */
//    size = frames*2*channel; /* 2 bytes/sample, 1 channels */
//    buffer = (char *) malloc(size);
 #if 0   
    while (1) {

        rc = read(fd, buffer, size);
        if (rc == 0) {
            fprintf(stderr, "end of file on input\n");
            break;
        } 
        else if (rc != size) {
            fprintf(stderr,"short read: read %d bytes\n", rc);
        }
        rc = snd_pcm_writei(handle, buffer, frames);
        if (rc == -EPIPE) {
            fprintf(stderr, "underrun occurred\n");
            err=snd_pcm_prepare(handle);
            if( err <0){
                fprintf(stderr, "can not recover from underrun: %s\n",snd_strerror(err));
            }
        
        } 
        else if (rc < 0) {
            fprintf(stderr,"error from writei: %s\n",snd_strerror(rc));
        }  
        else if (rc != (int)frames) {
            fprintf(stderr,"short write, write %d frames\n", rc);
        }
    }
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
#endif
    return OK;
}

int sound_driver_playframe(void *data, int frames)
{
    int rc,ret,err;
    rc = snd_pcm_writei(handle, data, frames);
    if (rc == -EPIPE) {
        fprintf(stderr, "underrun occurred\n");
        err=snd_pcm_prepare(handle);
        if( err <0){
            fprintf(stderr, "can not recover from underrun: %s\n",snd_strerror(err));
            ret=FAIL;
        }

    }
    else if (rc < 0) {
        fprintf(stderr,"error from writei: %s\n",snd_strerror(rc));
        ret=FAIL;
    }
    else if (rc != (int)frames) {
        fprintf(stderr,"short write, write %d frames\n", rc);
        ret=FAIL;
    }
    return OK;
}

int sound_driver_deinit()
{
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    return OK;
}

/*gpio */
int gpio_init()
{

    return OK;
}

int gpio_set_cfgpin(int pin, int val)
{
    return OK;
}

/*
读取 GPIO 状态,调用此函数自动设置对应 pin 为 input,默认 Pull_up
*/

int gpio_input(int pin)
{
    return OK;
}

int gpio_output(int pin, int val)
{
    return OK;
}

int gpio_deinit()
{
    return OK;
};
