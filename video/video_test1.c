#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>

static long int screensize = 0;


//14byte文件头
typedef struct
{
    char cfType[2];//文件类型，"BM"(0x4D42)
    long cfSize;//文件大小（字节）
    long cfReserved;//保留，值为0
    long cfoffBits;//数据区相对于文件头的偏移量（字节）
}__attribute__((packed)) BITMAPFILEHEADER;
//__attribute__((packed))的作用是告诉编译器取消结构在编译过程中的优化对齐

//40byte信息头
typedef struct
{
    char ciSize[4];//BITMAPFILEHEADER所占的字节数
    long ciWidth;//宽度
    long ciHeight;//高度
    char ciPlanes[2];//目标设备的位平面数，值为1
    int ciBitCount;//每个像素的位数
    char ciCompress[4];//压缩说明
    char ciSizeImage[4];//用字节表示的图像大小，该数据必须是4的倍数
    char ciXPelsPerMeter[4];//目标设备的水平像素数/米
    char ciYPelsPerMeter[4];//目标设备的垂直像素数/米
    char ciClrUsed[4]; //位图使用调色板的颜色数
    char ciClrImportant[4]; //指定重要的颜色数，当该域的值等于颜色数时（或者等于0时），表示所有颜色都一样重要
}__attribute__((packed)) BITMAPINFOHEADER;

typedef struct
{
    unsigned short blue;
    unsigned short green;
    unsigned short red;
    unsigned short reserved;
}__attribute__((packed)) PIXEL;//颜色模式RGB

BITMAPFILEHEADER FileHead;
BITMAPINFOHEADER InfoHead;

static char *fbp = 0;
static int xres = 0;
static int yres = 0;
static int bits_per_pixel = 0;
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))

int show_bmp();
int show_bmp2();
int fbfd = 0;
static void fb_update0(struct fb_var_screeninfo *vi)   //将要渲染的图形缓冲区的内容绘制到设备显示屏来
{  
    vi->yoffset = 0;  
    ioctl(fbfd, FBIOPAN_DISPLAY, vi);  
}  

static void fb_update1(struct fb_var_screeninfo *vi)   //将要渲染的图形缓冲区的内容绘制到设备显示屏来
{  
    vi->yoffset = yres;  
    ioctl(fbfd, FBIOPAN_DISPLAY, vi);  //FBIOPUT_VSCREENINFO
}  


int width, height;

static int cursor_bitmpa_format_convert(char *dst,char *src){
    int i ,j ;
    char *psrc = src ;
    char *pdst = dst;
    char *p = psrc;
    int value = 0x00;

    /* 由于bmp存储是从后面往前面，所以需要倒序进行转换 */
    pdst += (width * height * 4);
    for(i=0;i<height;i++){
        p = psrc + (i+1) * width * 3;
        for(j=0;j<width;j++){
            pdst -= 4;
            p -= 3;
            pdst[0] = p[0];
            pdst[1] = p[1];
            pdst[2] = p[2];
            //pdst[3] = 0x00;

            value = *((int*)pdst);
            value = pdst[0];
            if(value == 0x00){
                pdst[3] = 0x00;
            }else{
                pdst[3] = 0xff;
            }
        }
    }

    return 0;
}
#if 0
int show_bmp(char *path,int num)
{
    int total_length = 0;
    
//    1111100000000000;
//    0000011111100000
//    0000000000011111
    int i=0;
    if(num==0)
    {
        for(i=0;i<screensize/4;i++)
        {
            ((short*)fbp)[i]=0xf800;
        }
    }else if(num==1)
    {
        for(i=0;i<screensize/4;i++)
        {
            ((short*)fbp+screensize/4)[i]=0x07e0;
        }
    }else
    {    
        for(i=0;i<screensize/4;i++)
        {
            ((short*)fbp)[i]=0x001f;
        }
    }
}
#else
int show_bmp(char *path,int num)
{
    FILE *fp;
    int rc;
    int line_x, line_y;
    long int location = 0, BytesPerLine = 0;
    char *bmp_buf = NULL;
    char *bmp_buf_dst = NULL;
    char * buf = NULL;
    int flen = 0;
    int ret = -1;
    int total_length = 0;

    printf("into show_bmp function_____________________________________________________________________________________\n");
    if(path == NULL)
    {
        printf("path Error,return");
        return -1;
    }
    printf("path = %s\n", path);
    fp = fopen( path, "rb" );
    if(fp == NULL){
        printf("load > cursor file open failed");
        return -1;
    }
    /* 求解文件长度 */
    fseek(fp,0,SEEK_SET);
    fseek(fp,0,SEEK_END);
    flen = ftell(fp);

    bmp_buf = (char*)calloc(1,flen - 54);
    if(bmp_buf == NULL){
        printf("load > malloc bmp out of memory!");
        return -1;
    }

    /* 再移位到文件头部 */
    fseek(fp,0,SEEK_SET);

    rc = fread(&FileHead, sizeof(BITMAPFILEHEADER),1, fp);
    if ( rc != 1)
    {
        printf("read header error!\n");
        fclose( fp );
        return( -2 );
    }

    //检测是否是bmp图像
    if (memcmp(FileHead.cfType, "BM", 2) != 0)
    {
        printf("it's not a BMP file\n");
        fclose( fp );
        return( -3 );
    }
    rc = fread( (char *)&InfoHead, sizeof(BITMAPINFOHEADER),1, fp );
    if ( rc != 1)
    {
        printf("read infoheader error!\n");
        fclose( fp );
        return( -4 );
    }
    width = InfoHead.ciWidth;
    height = InfoHead.ciHeight;
    printf("FileHead.cfSize =%d byte\n",FileHead.cfSize);
    printf("flen = %d", flen);    
    printf("width = %d, height = %d\n", width, height);
    total_length = width * height *3;

    printf("total_length = %d\n", total_length);
    //跳转的数据区
    fseek(fp, FileHead.cfoffBits, SEEK_SET);
    printf(" FileHead.cfoffBits = %d\n",  FileHead.cfoffBits);
    printf(" InfoHead.ciBitCount = %d\n",  InfoHead.ciBitCount);  
    //每行字节数
    buf = bmp_buf;
    while ((ret = fread(buf,1,total_length,fp)) >= 0) {
        if (ret == 0) {
            usleep(100);
            continue;
        }
        printf("ret = %d\n", ret);
        buf = ((char*) buf) + ret;
        total_length = total_length - ret;
        if(total_length == 0)break;
    }

    total_length = width * height * 4;
    printf("total_length = %d\n", total_length);
    bmp_buf_dst = (char*)calloc(1,total_length);
    if(bmp_buf_dst == NULL){
        printf("load > malloc bmp out of memory!\n");
        return -1;
    }
    printf("cursor_bitmpa_format_convert\n");
    cursor_bitmpa_format_convert(bmp_buf_dst, bmp_buf);
    printf("cursor_bitmpa_format_convert end\n");    
//  if(screensize)
    printf("MIN=%d\n",MIN(total_length,screensize/2));
    printf("xres=%d,yres=%d\n",xres,yres);
    printf("xres*yres*4=%d\n",xres*yres*4);
    if(num==0)
        memcpy(fbp,bmp_buf_dst,MIN(total_length,screensize/2));
    else
        memcpy(fbp+xres*yres*4,bmp_buf_dst,MIN(total_length,screensize/2));

    printf("show logo\n");
    return 0;
}
#endif

int show_picture(int fd, char *path)
{

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    struct fb_bitfield red;
    struct fb_bitfield green;
    struct fb_bitfield blue;
    printf("Enter show_logo\n");
    //打开显示设备
retry1:
    fbfd = fd;//open("/dev/graphics/fb0", O_RDWR);
    printf("fbfd = %d\n", fbfd);
    if (fbfd == -1)
    {
        printf("Error opening frame buffer errno=%d (%s)",
                errno, strerror(errno));
        goto retry1;
    }

    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))
    {
        printf("Error：reading fixed information.\n");
        return -1;
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))
    {
        printf("Error: reading variable information.\n");
        return -1;
    }

    printf("R:%d,G:%d,B:%d \n", vinfo.red, vinfo.green, vinfo.blue );
    printf("xres_virtual:%d,yres_virtual:%d\n",vinfo.xres_virtual,vinfo.yres_virtual);

    printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );

    printf("red.offset=%u,length=%u\n",vinfo.red.offset,vinfo.red.length);
    printf("green.offset=%u,length=%u\n",vinfo.green.offset,vinfo.green.length);
    printf("blue.offset=%u,length=%u\n",vinfo.blue.offset,vinfo.blue.length);
    
    xres = vinfo.xres;
    yres = vinfo.yres;
    bits_per_pixel = vinfo.bits_per_pixel;

    //计算屏幕的总大小（字节）
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    printf("screensize=%d byte\n",screensize);
    screensize=screensize*2;
    //对象映射
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if ((int)fbp == -1)
    {
        printf("Error: failed to map framebuffer device to memory.\n");
        return -1;
    }

    printf("sizeof file header=%d\n", sizeof(BITMAPFILEHEADER));

    show_bmp("/nfs/test1.bmp",1);
    fb_update1(&vinfo);        

    //显示图像
    while(1){
//        show_bmp("/nfs/test.bmp",0);
//        fb_update0(&vinfo);
//        sleep(1);
//        show_bmp("/nfs/test1.bmp",1);
//        fb_update1(&vinfo);        
//        memset(fbp,0x8f,screensize);
//        sleep(1);
//        show_bmp("/nfs/test2.bmp",2);
//        fb_update0(&vinfo);        
  //        memset(fbp,0x8f,screensize);
        sleep(100000);

  }

    //删除对象映射
    munmap(fbp, screensize);
    //close(fbfd);
    printf("Exit show_logo\n");
    return 0;
}




int main()
{
    int fd=open("/dev/fb0", O_RDWR);
    if(fd<0)
    {
        printf("open fb0 error\n");
    }
    show_picture(fd,"/nfs/test.bmp");

    return 0;
}
