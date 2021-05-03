#ifndef _RECV_ANA_H_
#define _RECV_ANA_H_

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <memory.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#endif

#define DEVICE_NAME_H2C_0 "/dev/xdma0_h2c_0"
#define DEVICE_NAME_C2H_0 "/dev/xdma0_c2h_0"
#define DEVICE_NAME_H2C_1 "/dev/xdma0_h2c_1"
#define DEVICE_NAME_C2H_1 "/dev/xdma0_c2h_1"

#define MAP_SIZE (0x7FFFFFFF) //8bit 2GB    ;max value of int
#define MAP_BYPASS_SIZE (4 * 1024)
#define IMG_RAM_POS (0)
#define RX_SIZE (50 * 1024 * 1024) //1byte 8bit

#define PACK_SIZE (RX_SIZE / 16)
#define CHANNEL_NUM (8)
#define CPU_CORE (4) //CPU核心数量,使用顺序从数值最大的核心开始分配,留下第一个核心不分配
#define CLIENT_NUM (7)

typedef struct board_head
{
    int board_type;//8
    int board_addr;//8
    int Ftype;//2
    int Error;//14
} BOARD_HEAD;
typedef struct frame_head
{
    int channel_id;//8
    int error;//6
    int Ftype;//2
    int length;//16
    //char timestamp_H[32];
    //char timestamp_L[32];
    long long timestamp;//64
} FRAME_HEAD;

//根据CPU创建和分配线程
void ptd_create(pthread_t *arg, int k, void *functionbody)
{
    int ret;

    pthread_attr_t attr;

    ret = pthread_attr_init(&attr); //初始化线程属性变量,成功返回0,失败-1
    if (ret < 0)
    {
        perror("Init attr fail");
        exit(1);
    }

    if (k != -1)
    { //k为-1时不使用核心亲和属性
        cpu_set_t cpusetinfo;
        CPU_ZERO(&cpusetinfo);
        CPU_SET((CPU_CORE - 1 - k), &cpusetinfo); //将core1加入到cpu集中,同理可以将其他的cpu加入

        ret = pthread_attr_setaffinity_np(&attr, sizeof(cpusetinfo), &cpusetinfo);
        if (ret < 0)
        {
            perror("Core set fail");
            exit(1);
        }
    }

    /* ret = pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);//PTHREAD_SCOPE_SYSTEM绑定;PTHREAD_SCOPE_PROCESS非绑定
	if(ret < 0) {
		perror("Setscope fail");
		exit(1);
	} */
    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //线程分离属性:PTHREAD_CREATE_JOINABLE（非分离）
    if (ret < 0)
    {
        perror("Detached fail");
        exit(1);
    }

    pthread_create(arg, &attr, (void *(*)(void *))functionbody, NULL);
    //printf("Id为%d的线程已创建完毕。", *arg);

    pthread_attr_destroy(&attr); //销除线程属性
}

//speed test pthread
void *speed_test(void *count, char *order)
{
    long *__cnt = (long *)count; //注释掉,使用global identifier代替
    struct timeval systime;
    long time0, time1, time2 = 0, cnt1 = 0;
    double speed0, speed1;
    int size_of_every_time = 1024; //1024kB
    gettimeofday(&systime, 0);
    time0 = systime.tv_sec * 1e6 + systime.tv_usec;
    while (1)
    {
        gettimeofday(&systime, 0);
        time1 = systime.tv_sec * 1e6 + systime.tv_usec - time0;
        while (time1 - time2 > 1)
        {
            if (*order != 'n')
            {
                //calculate every second
                //speed in one seconds
                speed0 = (double)(*__cnt - cnt1) /
                         (double)((time1 - time2) * 1e6 * size_of_every_time);
                //total average speed
                speed1 = (double)(*__cnt) / (double)(time1 * 1e6 * size_of_every_time);
                printf("Average speed: %lfMB/s, second speed: %lfMB/s\n",
                       speed0, speed1);
            }
            time2 = time1;
            cnt1 = *__cnt;
        }
    }
}

/*开中断*/
int open_event(char *devicename)
{
    int fd;
    fd = open(devicename, O_RDWR | O_SYNC);
    if (fd == -1)
    {
        printf("open event error\n");
        return -1;
    }
    printf("open event successful!\n");
    return fd;
}

/*获取用户中断*/
int read_event(int fd)
{
    int val;
    //printf("Reading\n");
    read(fd, &val, 4);
    //printf("event=%x\n",val);
    return val;
}
/*open bypass devies*/
static int open_control(char *filename)
{
    int fd;
    fd = open(filename, O_RDWR | O_SYNC);
    if (fd == -1)
    {
        printf("open control error\n");
        return -1;
    }
    return fd;
}

//map bypass devies addrs
static void *mmap_control(int fd, long mapsize)
{
    void *vir_addr;
    vir_addr = mmap(0, mapsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    return vir_addr;
}

//write bypass devies
static void write_control(void *base_addr, int offset, uint32_t val)
{
    //uint32_t writeval = htoll(val);
    *((uint32_t *)(base_addr + offset)) = val;
}

//read bypass deives
static uint32_t read_control(void *base_addr, int offset)
{
    uint32_t read_result = *((uint32_t *)(base_addr + offset));
    //read_result = ltohl(read_result);
    return read_result;
}

unsigned int getbitr_fun(unsigned int *in, int lo, int si){
    unsigned int ret = 0;
    ret = (unsigned int)(ldexp(1, si) - 1) & *in >> lo;
    return ret;
}

//useless
unsigned int getbitl_fun(unsigned int *in, int lo, int si)
{
    unsigned int ret = 0;
    //ret =  - 1) & *in << lo;
    return ret;
}

long long bit_head_read(unsigned int *in_, char sig_0){
    if(in_ == NULL){
        printf("Bit read error!\n");
        return 0;
    }
    long long ret;
    switch (sig_0)
    {
    case 'c':
        return getbitr_fun(in_, 24, 8);
        break;

    case 'l':
        return getbitr_fun(in_, 0, 16);
        break;

    case 't':
        ret = getbitr_fun(in_ + 2, 0, 32);
        ret = getbitr_fun(in_ + 1, 0, 32) | ret << 32;
        return ret;
        break;

    case 'b':
        printf("**********BOARD INFO**********\n");
        printf("Board type:%d \nBoard addr:%d\nBoard Ftype:%d\nBoard Error:%d\n\n",
         getbitr_fun(in_, 24, 8), getbitr_fun(in_, 16, 8), 
         getbitr_fun(in_, 14, 2), getbitr_fun(in_, 0, 14));
        return 0;
        break;

    case 'f':
        printf("**********FRAME INFO**********\n");
        printf("channel_id: %d\nError: %d\nFtype: %d\nLength: %d\n",
         getbitr_fun(in_, 24, 8), getbitr_fun(in_, 18, 6), getbitr_fun(in_, 16, 2), 
         getbitr_fun(in_, 0, 16));
        ret = getbitr_fun(in_ + 2, 0, 32);
        ret = getbitr_fun(in_ + 1, 0, 32) | ret << 32;
        printf("Timestamp: %lld\n\n", ret);

    default:
        printf("sig_0 error!\n");
        return 0;
        break;
    }
}

//need adc data int
unsigned int bit_data_read(unsigned int *in_, char sig_1, int d)
{
    if (in_ == NULL)
    {
        printf("Bit read error!\n");
        return 0;
    }
    int ret;
    if(d == 0){
        switch (sig_1)
        {
        case 'l':
            return getbitr_fun(in_, 16, 12);
            break;

        case 'c':
            return getbitr_fun(in_, 28, 4);

        default:
            return 0;
            break;
        }
    }else{
        switch (sig_1)
        {
        case 'l':
            return getbitr_fun(in_, 0, 12);
            break;

        case 'c':
            return getbitr_fun(in_, 12, 4);

        default:
            return 0;
            break;
        }
    }
}

double bit_float_read(unsigned int *in_, int d)
{
    if (in_ == NULL)
    {
        printf("Bit read error!\n");
        return 0;
    }
    int ret;
    if (d == 0)
    {
        ret = getbitr_fun(in_, 16, 12);
        return (-5. + (double)ret * ((double)(ldexp(1, 12) - 1) / (double)10));
    }
    else
    {
        ret = getbitr_fun(in_, 0, 12);
        return (-5. + (double)ret * ((double)(ldexp(1, 12) - 1) / (double)10));
    }
}

