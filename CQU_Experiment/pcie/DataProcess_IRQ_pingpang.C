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

#define DEVICE_NAME_H2C_0 "/dev/xdma0_h2c_0"
#define DEVICE_NAME_C2H_0 "/dev/xdma0_c2h_0"
#define DEVICE_NAME_H2C_1 "/dev/xdma0_h2c_1"
#define DEVICE_NAME_C2H_1 "/dev/xdma0_c2h_1"

#define MAP_SIZE (0x7FFFFFFF) //8bit 2GB
#define MAP_BYPASS_SIZE (4 * 1024)
#define IMG_RAM_POS (0)
#define RX_SIZE (10 * 1024 * 1024) //1byte 8bit

#define PACK_SIZE (4 * 1024)
#define CHANNEL_NUM (8)
#define CPU_CORE (4) //CPU核心数量,使用顺序从数值最大的核心开始分配,留下第一个核心不分配
#define CLIENT_NUM (7)

int work;
int write;

int c2h_fd;
int h2c_fd;
int control_fd;
void *control_base;
int interrupt_fd;
int read_end_addr;

unsigned int pData[2][RX_SIZE / 4]; //10M / 4 * 32 bit     count
                                    //10M / 4 * 4 MB       size

static sem_t int_sem_rxa; //init 1
static sem_t int_sem_rxb; //init 0
static sem_t int_sem_rxc; //init 1

//准备发往各个客户端的二维数组
unsigned int pack_send[CHANNEL_NUM][RX_SIZE / CLIENT_NUM * 2];

//socket所需变量
int acfd[CLIENT_NUM];
int port = 10000;
int socket_fd;
struct sockaddr_in clientaddr[CLIENT_NUM];

int total_count;

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

unsigned int getbit_fun(unsigned int *in, int lo, int si)
{
    unsigned int ret = 0;
    ret = (unsigned int)(ldexp(1, si) - 1) & *in >> lo;
    return ret;
}

long long bit_head_read(void *in, char sig_0)
{
    if (in == NULL)
    {
        printf("Bit read error!\n");
        return 0;
    }
    unsigned int *in_ = (unsigned int *)in;
    switch (sig_0)
    {
    case 'c':
        return getbit_fun(in_, 24, 8);
        break;

    case 'l':
        return getbit_fun(in_, 0, 16);
        break;

    case 't':
        long long ret = getbit_fun(in_ + 2, 0, 32);
        ret = getbit_fun(in_ + 1, 0, 32) | ret << 32;
        return ret;
        break;

    case 'b':
        printf("**********BOARD INFO**********\n");
        printf("Board type:%d \nBoard addr:%d\nBoard Ftype:%d\nBoard Error:%d\n\n",
               getbit_fun(in_, 24, 8), getbit_fun(in_, 16, 8),
               getbit_fun(in_, 14, 2), getbit_fun(in_, 0, 14));
        return 0;
        break;

    case 'f':
        printf("**********FRAME INFO**********\n");
        printf("channel_id: %d\nError: %d\nFtype: %d\nLength: %d\n",
               getbit_fun(in_, 24, 8), getbit_fun(in_, 18, 6), getbit_fun(in_, 16, 2),
               getbit_fun(in_, 0, 16));
        long long ret = getbit_fun(in_ + 2, 0, 32);
        ret = getbit_fun(in_ + 1, 0, 32) | ret << 32;
        printf("Timestamp: %lld\n\n", ret);

    default:
        printf("sig_0 error!\n");
        return 0;
        break;
    }
}

unsigned int bit_data_read(unsigned int *in_, char sig_1, int d)
{
    if (in_ == NULL)
    {
        printf("Bit read error!\n");
        return 0;
    }
    int ret;
    if (d == 0)
    {
        switch (sig_1)
        {
        case 'l':
            return getbit_fun(in_, 16, 12);
            break;

        case 'c':
            return getbit_fun(in_, 28, 4);

        default:
            return 0;
            break;
        }
    }
    else
    {
        switch (sig_1)
        {
        case 'l':
            return getbit_fun(in_, 0, 12);
            break;

        case 'c':
            return getbit_fun(in_, 12, 4);

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
        ret = getbit_fun(in_, 16, 12);
        return (-5. + (double)ret * ((double)(ldexp(1, 12) - 1) / (double)10));
    }
    else
    {
        ret = getbit_fun(in_, 0, 12);
        return (-5. + (double)ret * ((double)(ldexp(1, 12) - 1) / (double)10));
    }
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

/*中断处理进程*/
void *event_process()
{
    interrupt_fd = open_event("/dev/xdma0_events_0"); //打开用户中断
    while (work == 1)
    {
        sem_wait(&int_sem_rxa);
        //printf("Start read interrupt !\n");
        read_event(interrupt_fd);                                //获取用户中断
        read_end_addr = (int)read_control(control_base, 0x0008); //read interrupt reg
        write_control(control_base, 0x0000, 0xFFFFFFFF);         //ack interrupt
        //printf("read_end_addr = %x \n",read_end_addr);
        //lseek(c2h_fd, read_end_addr, SEEK_SET);
        //read(c2h_fd, pData[0], 32*1024);
        //cnt = cnt +1;
        //printf("cnt = %d\n",cnt);
        //printf("Stop read!\n");
        sem_post(&int_sem_rxb); //release signal
    }
    pthread_exit(0);
}

void *rx_process()
{
    struct timeval sstime;
    long time0;

    while (work == 1)
    {
        sem_wait(&int_sem_rxb);
        //while(read all of the buf :50MB)
        printf("read_end_addr = %x \n", read_end_addr);
        lseek(c2h_fd, read_end_addr, SEEK_SET);
        read(c2h_fd, pData[0], RX_SIZE);
        printf("cnt: %d\n", total_count);
        total_count++;
        //sem_post(&int_sem_rxc);//处理数据需要的信号
        sem_post(&int_sem_rxa); //当有两个数组交替读取时,条件执行post:a
                                //因为暂时不需要交替,无条件执行post:a
        //while break;
        //sem_post(&int_sem_rxa);
    }
}

//open such device func:
void dev_open_fun()
{
    control_fd = open_control("/dev/xdma0_bypass");           //打开bypass字符设备
    control_base = mmap_control(control_fd, MAP_BYPASS_SIZE); //获取bypass映射的内存地址

    c2h_fd = open(DEVICE_NAME_C2H_0, O_RDONLY | O_NONBLOCK); //打开pcie c2h设备
    if (c2h_fd == -1)
    {
        printf("PCIe c2h device open failed!\n");
    }
    else
        printf("PCIe c2h device open successful!\n");

    mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, c2h_fd, 0);

    printf("c2h device Memory map successful!\n");

    h2c_fd = open(DEVICE_NAME_H2C_0, O_WRONLY | O_NONBLOCK); //打开pcie  h2c设备
    if (h2c_fd == -1)
    {
        printf("PCIe h2c device open failed!\n");
    }
    else
        printf("PCIe h2c device open successful!\n");

    mmap(0, MAP_SIZE, PROT_WRITE, MAP_SHARED, h2c_fd, 0);

    printf("h2c device Memory map successful!\n");
}

int main()
{
    sem_init(&int_sem_rxa, 0, 1);
    sem_init(&int_sem_rxb, 0, 0);
    sem_init(&int_sem_rxc, 0, 1);

    char sig;
    unsigned int pos = 0;
    unsigned int cnontrol_3;

    pthread_t event_thread, rx_thread;
    pthread_t part_ptd;

    dev_open_fun();

    work = 1;

    pthread_create(&event_thread, NULL, (void *(*)(void *))event_process, NULL);
    pthread_create(&rx_thread, NULL, (void *(*)(void *))rx_process, NULL);

    while (sig != 'o')
    {
        sig = getchar();
        switch (sig)
        {
        case 'w':
            lseek(c2h_fd, pos, SEEK_SET);
            read(c2h_fd, pData[0], 4 * 1024);
            printf("Read data successful!\n");

            for (int i = 0; i < 1024; i++)
            {
                printf("The data of address %d is %x \n", i, pData[0][i]);
            }
            break;

        case 'e':
            total_count = 0;
            break;

        case 'r':
            write_control(control_base, 0x0000, 0xFFFFFFFF);
            break;

        case 't':
            write_control(control_base, 0x0004, 0xFFFFFFFF);
            break;

        case 'y':
            cnontrol_3 = read_control(control_base, 0x0008);
            printf("read_end_addr = %x \n", cnontrol_3);
            break;

        case 'u':
            pos = pos + 1024;
            printf("test_read_end_addr = %x \n", pos);
            break;

        case 'i':
            pos = 0;
            printf("test_read_end_addr = %x \n", pos);
            break;

        case 'g':
            memcpy(pData[1], pData[0], sizeof(pData[1]));
            FILE *fp;
            fp = fopen("data.log", "w+");
            if (fp == NULL)
            {
                printf("File open failed!");
                break;
            }
            fwrite(pData[1], 1, sizeof(pData[1]), fp);
            fclose(fp);
            memset(pData[1], '0', sizeof(pData[1]));
            break;
        }
    }
    work = 0;

    close(c2h_fd);
    close(h2c_fd);
    close(control_fd);
    close(interrupt_fd);
    return 0;
}