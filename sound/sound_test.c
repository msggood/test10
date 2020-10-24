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

static snd_pcm_t *g_pcm_handle;
static char *g_buffer;
static int g_size;
static FILE* fp=NULL;


// 定义音乐全局结构体，具体该定义什么参考 https://www.cnblogs.com/ranson7zop/p/7657874.html 表3
// int 由uint32_t代替，short 由uint16_t代替，因为在跨平台后有可能不兼容，类型长度不一致，使用统一的类型
struct WAV_HEADER
{

    char        chunk_id[4]; // riff 标志号
    uint32_t    chunk_size; // riff长度
    char        format[4]; // 格式类型(wav)

    char        sub_chunk1_id[4]; // fmt 格式块标识
    uint32_t    sub_chunk1_size; // fmt 长度 格式块长度。
    uint16_t    audio_format; // 编码格式代码                                   常见的 WAV 文件使用 PCM 脉冲编码调制格式,该数值通常为 1
    uint16_t    num_channels; // 声道数                                     单声道为 1,立体声或双声道为 2
    uint32_t    sample_rate; // 采样频率                                    每个声道单位时间采样次数。常用的采样频率有 11025, 22050 和 44100 kHz。
    uint32_t    byte_rate; // 传输速率                                      该数值为:声道数×采样频率×每样本的数据位数/8。播放软件利用此值可以估计缓冲区的大小。
    uint16_t    block_align; // 数据块对齐单位                                  采样帧大小。该数值为:声道数×位数/8。播放软件>需要一次处理多个该值大小的字节数据,用该数值调整缓冲区。
    uint16_t    bits_per_sample; // 采样位数                                存储每个采样值所用的二进制数位数。常见的位数有 4>、8、12、16、24、32

    char        sub_chunk2_id[4]; // 数据  不知道什么数据
    uint32_t    sub_chunk2_size; // 数据大小

} wav_header;
int wav_header_size; // 接收wav_header数据结构体的大小


// 想要定义函数必须要先声明函数，在头文件中声明了
FILE* open_file(const char *path_name)
{
    // 通过fopen函数打开音乐文件
    FILE* fp = fopen(path_name, "rb");
    // 判断文件是否为空
    if(fp == NULL){
        printf("music file is NULL \n");
        fclose(fp);
        exit(1);
    }
    // 把文件指针定位到文件的开头处
    fseek(fp, 0, SEEK_SET);

    // 读取文件，并解析文件头获取有用信息
    wav_header_size = fread(&wav_header, 1, sizeof(wav_header), fp);
    printf("wav文件头结构体大小：   %d          \n", wav_header_size);
    printf("RIFF标志：      \t      %c%c%c%c    \n", wav_header.chunk_id[0], wav_header.chunk_id[1], wav_header.chunk_id[2],wav_header.chunk_id[3]);
    printf("文件大小：      \t      %d          \n", wav_header.chunk_size);
    printf("文件格式：      \t      %c%c%c%c    \n", wav_header.format[0], wav_header.format[1], wav_header.format[2], wav_header.format[3]);
    printf("格式块标识：    \t      %c%c%c%c    \n", wav_header.sub_chunk1_id[0], wav_header.sub_chunk1_id[1], wav_header.sub_chunk1_id[2], wav_header.sub_chunk1_id[3]);
    printf("格式块长度：    \t      %d          \n", wav_header.sub_chunk1_size);
    printf("编码格式代码:   \t      %d          \n", wav_header.audio_format);
    printf("声道数:         \t      %d          \n", wav_header.num_channels);
    printf("采样频率:       \t      %d          \n", wav_header.sample_rate);
    printf("传输速率：      \t      %d          \n", wav_header.byte_rate);
    printf("数据块对齐单位: \t      %d          \n", wav_header.block_align);
    printf("采样位数(长度): \t      %d          \n", wav_header.bits_per_sample);
    return fp;

}



int sound_driver_init(char* outdev, int samples, int channel)
{

	int rc;
	unsigned int val;
	int dir;
	snd_pcm_uframes_t frames;

	int err;

	//以播放模式打开设备
	rc = snd_pcm_open(&g_pcm_handle, outdev,SND_PCM_STREAM_PLAYBACK, 0);
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
	err=snd_pcm_hw_params_any(g_pcm_handle, params);
	if (err < 0)
	{
		fprintf(stderr, "Can not configure this PCM device: %s\n",snd_strerror(err));
		exit(1);
	}

	//设置多路数据在buffer中的存储方式
	//SND_PCM_ACCESS_RW_INTERLEAVED每个周期(period)左右声道的数据交叉存放
	err=snd_pcm_hw_params_set_access(g_pcm_handle, params,SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) 
	{
		fprintf(stderr,"Failed to set PCM device to interleaved: %s\n",snd_strerror(err));
		exit(1);
	}

	//设置16位采样格式，S16为有符号16位,LE是小端模式
	err=snd_pcm_hw_params_set_format(g_pcm_handle, params,SND_PCM_FORMAT_S16_LE);
	if (err < 0)
	{
		fprintf(stderr,"Failed to set PCM device to 16-bit signed PCM: %s\n",snd_strerror(err));
		exit(1);
	}

	//设置声道数,双声道
	err=snd_pcm_hw_params_set_channels(g_pcm_handle, params, channel);
	if (err < 0)
	{
		fprintf(stderr, "Failed to set PCM device to mono: %s\n",snd_strerror(err));
		exit(1);
	}

	val = samples;
	//设置采样率,如果采样率不支持，会用硬件支持最接近的采样率
	err=snd_pcm_hw_params_set_rate_near(g_pcm_handle, params,&val, &dir);
	if (err < 0) 
	{
		fprintf(stderr, "Failed to set PCM device to sample rate =%d: %s\n",val,snd_strerror(err));
		exit(1);
	}

	unsigned int period_time;
	period_time = 26315;
	//    period_time = 1024;
	//设置周期时间
	err = snd_pcm_hw_params_set_period_time_near(g_pcm_handle, params, &period_time, 0);
	if (err < 0) 
	{
		fprintf(stderr, "Failed to set PCM device to period time =%d: %s\n",period_time,snd_strerror(err));
		exit(1);
	}

	//让这些参数作用于PCM设备
	rc = snd_pcm_hw_params(g_pcm_handle, params);
	if (rc < 0) 
	{
		fprintf(stderr,"unable to set hw parameters: %s\n",snd_strerror(rc));
		exit(1);
	}

	//    snd_pcm_hw_params_get_period_size(params, &frames,&dir);


	snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    snd_pcm_hw_params_free(params);

	g_size = frames * 2 * channel;
	g_buffer = (char*)malloc(g_size);
	if (!g_buffer) {
		fprintf(stderr, "Not enough Memory!\n");
		return -1;
	}
	return OK;
}



int sound_driver_playframe(void *data, int frames)
{
	int rc,ret,err;
    ret=OK;
	rc = snd_pcm_writei(g_pcm_handle, data, frames);
	if (rc == -EPIPE) {
		fprintf(stderr, "underrun occurred\n");
		err=snd_pcm_prepare(g_pcm_handle);
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
	return ret;
}

int sound_driver_deinit()
{
	snd_pcm_drain(g_pcm_handle);
	snd_pcm_close(g_pcm_handle);
    if(g_buffer)
    {
        free(g_buffer);
        g_buffer=NULL;
    }
	return OK;
}


static void play_wav(char *name,int sample_rate, int channels)
{
	int num_read;
    int frames;
    
    if (!fp) {
        fprintf(stderr, "failed to open '%s'\n", name);
        return;
    }

	do {
		num_read = fread(g_buffer, 1, g_size, fp);
		if (num_read > 0) {
            frames=snd_pcm_bytes_to_frames(g_pcm_handle, num_read);
            sound_driver_playframe(g_buffer,frames);
		}
	} while (num_read > 0);

}

//char * dev_name="default";
char * dev_name="plughw:1,0";

int main(int argc, char *argv[])
{
    fp=open_file(argv[1]);
	sound_driver_init(dev_name,wav_header.sample_rate,wav_header.num_channels);
	play_wav(argv[1],wav_header.sample_rate,wav_header.num_channels);
	while(1)
		sleep(1);
	sound_driver_deinit();
    fclose(fp);
	return 0;
}

