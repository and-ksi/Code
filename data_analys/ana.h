#ifndef _ANA_H_
#define _ANA_H_

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <memory.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#define __USE_GNU
#include <pthread.h>
#include <sched.h>
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
#include <dirent.h>

#define DEVICE_NAME_H2C_0 "/dev/xdma0_h2c_0"
#define DEVICE_NAME_C2H_0 "/dev/xdma0_c2h_0"
#define DEVICE_NAME_H2C_1 "/dev/xdma0_h2c_1"
#define DEVICE_NAME_C2H_1 "/dev/xdma0_c2h_1"

#define MAP_SIZE (0x7FFFFFFF) //8bit 2GB    ;max value of int
#define MAP_BYPASS_SIZE (4 * 1024)
#define IMG_RAM_POS (0)
#define RX_SIZE (4 * (BOARD_NUM) * 1024) //1byte 8bit
#define BOARD_NUM (1280 * 2)//5MB
#define CPU_CORE_NUM = (4)

#define PACK_SIZE (RX_SIZE / 4)

//ana data
#define SQRT_SIZE (100)          //mm
#define INTERVAL_NUM (33)        //99mm
#define DELAY_EVERY_INTERVAL (4) //ns
#define SIZE_EVERY_INTERVAL (3)  //mm
#define FREQUENCY (125)          //MHz
#define PERIOD_CYCLE (8)         //ns    ^^^^^   与频率相关联

#define PRO_FREQUENCY (0.2)      //MHz
#define PRO_PERIOD_CYCLE (5000 / 8 - 100)

#define ATTENUATION_COEFFICIENT (1.04)
#define DELAY_TIME (500)   //ns   恒比定时延迟时间
#define DELAY_MAX (8 * 50) //判断数据有效,上下正比室触发时间最大差值 \
                           //8ns * 50(48个数据)

#define ENERGY_OF_MOUN (105.7)

typedef struct location_timestamp
{
    int m_location;
    long long m_timestamp;
    int m_length;
    int m_channel;
}LOCA_TIME;

typedef struct speed_time{
    long *count;
    int *speed;
    int size;//kB
    FILE **m_fp;
}SPEED_T;

//根据CPU创建和分配线程
void ptd_create(pthread_t *arg, int k, void *functionbody, void *m_arg, int m)
{
    int ret;

    pthread_attr_t attr;

    ret = pthread_attr_init(&attr); //初始化线程属性变量,成功返回0,失败-1
    if (ret < 0)
    {
        perror("Init attr fail");
        exit(1);
    }

    if (k != 0)
    { //k为-1时不使用核心亲和属性
        cpu_set_t cpusetinfo;
        CPU_ZERO(&cpusetinfo);
        CPU_SET(k, &cpusetinfo); //将core1加入到cpu集中,同理可以将其他的cpu加入

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
    if(m != 0){
        ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); //线程分离属性:PTHREAD_CREATE_JOINABLE（非分离）
        if (ret < 0)
        {
            perror("Detached fail");
            exit(1);
        }
    }

    pthread_create(arg, &attr, functionbody, m_arg);
    //printf("debug: %d的线程已创建完毕。", k);

    pthread_attr_destroy(&attr); //销除线程属性
}

//speed test pthread
void *speed_test(void *ml)
{
    SPEED_T *speed_t = (SPEED_T *)ml;
    long *__cnt = speed_t->count; //注释掉,使用global identifier代替
    struct timeval systime;
    long time0, time1, time2 = 0, cnt1 = 0;
    double speed0, speed1;
    int size_of_every_time = speed_t->size / 1024; //1024kB
    gettimeofday(&systime, 0);
    time0 = time2 = systime.tv_sec;
    while (1)
    {
        gettimeofday(&systime, 0);
        time1 = systime.tv_sec;
        if (time1 - time2 > 1)
        {
            //calculate every second
            //speed in one seconds
            *(speed_t->speed + 0) = (double)(*__cnt - cnt1) /
                                    (double)((time1 - time2) * 1e6 * size_of_every_time);
            //total average speed
            *(speed_t->speed + 1) = (double)(*__cnt) / (double)(time1 * 1e6 * size_of_every_time);
            time2 = time1;
            cnt1 = *__cnt;
            //fprintf(*(speed_t->m_fp), "speed now = %d   speed average = %d\n", *(speed_t->speed + 0), *(speed_t->speed + 1));
            printf("speed now = %d   speed average = %d\n", *(speed_t->speed + 0), *(speed_t->speed + 1));
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
    *((uint32_t *)((uint32_t *)base_addr + offset)) = val;
}

//read bypass deives
static uint32_t read_control(void *base_addr, int offset)
{
    uint32_t read_result = *((uint32_t *)((uint32_t *)base_addr + offset));
    //read_result = ltohl(read_result);
    return read_result;
}

long long bit_time_read(unsigned int *in_)
{
    //long long ret = *(in_ + 1);
    //return ((ret << 32) | *(in_ + 2));

    long long ret = 0, ret1, ret2, ret3;
    int i, n = 0;
    for(i = 1;; i++){
        ret1 = ((*(in_ + i) & 0xffff0000) >> 16);
        ret2 = *(in_ + i) & 0x0000ffff;
        if (ret1 == 0xdddd || ret1 == 0xcccc || ret1 == 0xf000 || ret1 == 0xf00f){
            n++;
        }else{
            break;
        }
        if (ret2 == 0xdddd || ret2 == 0xcccc || ret2 == 0xf000 || ret2 == 0xf00f){
            n++;
        }else{
            break;
        }
    }

    if(n <= 2 || n > 4){
        return -1;
    }

    for(int k = i; k < 3; k++){
        ret1 = ((*(in_ + k) & 0xffff0000) >> 16);
        ret2 = *(in_ + k) & 0x0000ffff;
        if(ret1 == 0xf00f || ret2 == 0xf00f || ret1 == 0xf000 || ret2 == 0xf000){
            return -1;
        }
    }

    if(n & 1 == 1){
        ret = (ret2 << 48);
        ret1 = *(in_ + i + 1);
        ret2 = ((*(in_ + i + 2) & 0xffff0000) >> 16);
        ret = (ret | (ret1 << 16) | ret2);
        return ret;
    }else{
        ret1 = *(in_ + i);
        ret = (ret1 << 32);
        return ret = ret | (*(in_ + i + 1));
    }
    /* ret = *(in_ + 1);
    return (*(in_ + 2) | (ret << 32)); */
}

int find_board_head(unsigned int *in_, int k, int len)
{
    if (len == 0)
    {
        len = 10;
    }
    for (int i = 0; i < len; i++)
    {
        if (*(in_ + i) == 0xf00f)
        {
            //printf("debug: find_board_head: ret = %d\n", i);
            return i;
        }
        if (k)
        {
            if (*(in_ - i) == 0xf00f)
            {
                return -i;
            }
        }
        if (*(in_ + i) == 0)
        {
            return 100;
        }
    }
    return 100;
}

int find_adc_head(unsigned int *in_, int k, int len)
{
    if (len == 0)
    {
        len = 10;
    }
    int ret, ret1;
    int i;
    for (i = 0; i < len; i++)
    {
        //printf("debug: %x  %x, %x  %x  %x  %x   %x  %x\n", (*(in_ + i) & 0x0000fff0), (*(in_ + i + 1) & 0x0000fff0), *(in_ + i - 2), *(in_ + i - 1), *(in_ + i), *(in_ + i + 1), *(in_ + i + 2), *(in_ + i + 3));
        if ((*(in_ + i) & 0x0000fff0) == 0x00003f00 && (*(in_ + i + 1) & 0x0000fff0) != 0x00003f00)
        {
            //printf("debug: find_adc_head: ret = %d\n", i);
            if ((*(in_ + i) & 0x000000ff) <= 8 && (*(in_ + i) >> 16) < 100)
            {
                return i;
            }
        }
        if ((*(in_ - i) & 0x0000fff0) == 0x00003f00 && (*(in_ - i + 1) & 0x0000fff0) != 0x00003f00)
        {
            if ((*(in_ - i) & 0x000000ff) <= 8 && (*(in_ - i) >> 16) < 100)
            {
                return -i;
            }
        }
    }
    return 100;
}

unsigned int bit_head_read(unsigned int *in_, char sig_0)
{
    if (in_ == NULL)
    {
        printf("Bit read error!\n");
        return 0;
    }
    unsigned int ret = 0;
    int i;
    switch (sig_0)
    {
/*     case 'T': //Board_type
        return bit_read(in_, 8, 8);
        break;

    case 'A': //Board_address
        return bit_read(in_, 8, 8);
        break;

    case 'Y': //Board_ftype
        return bit_read(in_, 8, 8);
        break;

    case 'E': //Board_error
        return bit_read(in_, 8, 8);
        break; */

    case 'c':

        return (*in_ & 0x000000ff) - 1;
        break;

    case 'e':

        return ((*in_ << 18) >> 26);
        break;

    case 'F':

        return (*in_ << 16) >> 30;
        break;

    case 'l':
        if ((*in_ >> 16) < 20)
        {
            for(int i = 1; i < 100; i++){
                ret = find_adc_head(in_ + i, 0, 90);
                if(ret < 100){
                    return i - 1;
                }
            }
            return 100;
        }
        else
        {
            return (*in_ >> 16);
        }
        break;

/*     case 'b':
        printf("**********BOARD INFO**********\n");
        printf("Board type:%x \nBoard addr:%x\nBoard Ftype:%x\nBoard Error:%x\n\n",
               bit_read(in_, 32, 8), bit_read(in_, 24, 8),
               bit_read(in_, 16, 2), bit_read(in_, 14, 14));
        return 0;
        break; */

    case 'f':
        printf("**********FRAME INFO**********\n");
        printf("channel_id: %02x\nError: %02x\nFtype: %x\nLength: %04d\nTimestamp: %llx\n\n",
               bit_head_read(in_, 'c'), bit_head_read(in_, 'e'),
               bit_head_read(in_, 'F'), bit_head_read(in_, 'l'), bit_time_read(in_));
        return 0;
        break;

    default:
        printf("sig_0 error!\n");
        return 0;
        break;
    }
}

//need adc data int pointer 指向需要读取的unsigned int
int bit_data_read(unsigned int *in_, char sig_1, int d)
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
        case 'd':

            return ((*in_ & 0x0fff0000) >> 16);

            break;

        case 'c':

            return *in_ & 0xf0000000;
            break;

        default:
            return 0;
            break;
        }
    }
    else
    {
        switch (sig_1)
        {
        case 'd':

            return *in_ & 0x00000fff;
            break;

        case 'c':

            return *in_ & 0x0000f000;
            break;

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
    ret = bit_data_read(in_, 'd', d);
    return (5. - (double)ret * (10. / (double)(ldexp(1, 12) - 1)));
}

void *open_error_log()
{
    if (NULL == opendir("log"))
    {
        mkdir("log", 0777);
    }
    time_t t;
    char buf[50];
    struct tm *lt;
    time(&t);           //获取Unix时间戳。
    lt = localtime(&t); //转为时间结构。
    sprintf(buf, "log/error_%d-%d_%d-%d-%d.log", lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);

    FILE *_error_log = fopen(buf, "w+");
    if (_error_log == NULL)
    {
        printf("Error log file open failed!\n");
        exit(1);
    }
    return _error_log;
}

void write_error_log(FILE **_fp, unsigned int *edata, int m_mark)
{
    FILE **error_fp = (FILE **)_fp;
    if(m_mark){
        for (int i = -50; i < 0; i++)
        {
            fprintf(*error_fp, "%x\n", *(edata + i));
        }
    }
    
    fprintf(*error_fp, "\n");
    for (int i = 0; i < 100; i++)
    {
        fprintf(*error_fp, "%x\n", *(edata + i));
    }
    //fclose(_fp);
}

void write_data_error_log(FILE **_fp, unsigned int *edata, int count, int m_mark)
{
    FILE **error_fp = (FILE **)_fp;
    fprintf(*error_fp, "\n");
    if (m_mark)
    {
        for (int i = - count / 2; i < 0; i++)
        {
            fprintf(*error_fp, "%x\n", *(edata + i));
        }
    }

    fprintf(*error_fp, "\n");
    for (int i = 0; i < count; i++)
    {
        fprintf(*error_fp, "%x\n", *(edata + i));
    }
    fflush(*error_fp);
    //fclose(_fp);
}

void *open_savelog(){
    if (NULL == opendir("datalog"))
    {
        mkdir("datalog", 0777);
    }

    char buf[100];
    FILE *save;

    time_t t;
    struct tm *lt;
    time(&t);           //获取Unix时间戳。
    lt = localtime(&t); //转为时间结构。
    sprintf(buf, "datalog/savelog_%d-%d_%d-%d-%d.log", lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
    save = fopen(buf, "w+");
    if (save == NULL)
    {
        printf("Log_save create failed!\n");
        exit(1);
    }
    printf("debug: open savelog success!\n");

    return save;
}

//if(*(in_ + i) & 0x00003f00 == 0x00003f00 && *(in_ + i + 1) & 0x00003f00 != 0x00003f00)

void take_head_to_struct(void *m_struct_, unsigned int *in_){

}

int get_latest_data(LOCA_TIME *m_list, long long time, int acount)
{
    int def[10];
    def[0] = abs((m_list + 0)->m_timestamp - time);
    def[1] = abs((m_list + 1)->m_timestamp - time);
    def[2] = abs((m_list + 2)->m_timestamp - time);
    for (int i = 1; i < 10; i++)
    {
        def[i] = abs((m_list + i)->m_timestamp - time);
        if (def[i - 1] <= def[i])
        {
            return i - 1;
        }
    }
    return -1;
}

//2500MHz时,延迟时间50次,20ns
//20MHz,延迟时间?
long long cfd_get_begintime(unsigned int *in_)
{
    if (in_ == NULL)
    {
        printf("Bit read error!\n");
        return 0;
    }

    unsigned int _cfd[1024];
    long long timestamp_ = bit_time_read(in_); //ns
    int _length = bit_head_read(in_, 'l');
    for (int i = 0; i < (2 * _length); i++)
    {
        _cfd[i] = bit_data_read(in_ + (i / 2), 'l', i & 1);
        if (i > DELAY_TIME / PERIOD_CYCLE - 1)
        {
            _cfd[i] += -ATTENUATION_COEFFICIENT * _cfd[i - DELAY_TIME / PERIOD_CYCLE];
            if (_cfd[i] == 0)
            {
                return timestamp_ + i * PERIOD_CYCLE;
            }
            else if (_cfd[i] * _cfd[i - 1] < 0)
            {
                return timestamp_ +
                       (long long)(PERIOD_CYCLE * (float)(_cfd[i] * (i - 1) / _cfd[i - 1]));
            }
        }
    }
    return 0;
}

//calculate position ;left+   right-
//take the center of square as zero point
//125MHz
double cal_position(unsigned int *s1, unsigned int *s2)
{
    long long t1 = bit_time_read(s1);
    long long t2 = bit_time_read(s2);

    return ((double)(t1 - t2) *
            (double)(SIZE_EVERY_INTERVAL / (2 * DELAY_EVERY_INTERVAL) * 1e-3));
}

//calculate kinetic energy
//需要传入指向指针数组的指针
//channel: X1, X2, Y1, Y2, X3, X4, Y3, Y4
//延迟块间4ns, 间距3mm, 从一端到另一端完整132ns
double cal_energy(unsigned int **_in)
{
    double s[4]; //s0, s1, s2, s3; xup, yup, xdo, ydo
    for (int i = 0; i < 4; i++)
    {
        s[i] = cal_position(*(_in + 2 * i), *(_in + 2 * i + 1));
    }
    int length_ = pow((s[0] - s[2]) * (s[0] - s[2]) + (s[1] - s[4]) * (s[1] - s[3]) + .25 * .25 , 1./3);
    long long t[8];
    for (int i = 0; i < 8; i++)
    {
        t[i] = cfd_get_begintime(*(_in + i));
    }
    double arrive_time[2];
    arrive_time[0] = (t[0] + t[1] + t[2] + t[3]) / 4;
    arrive_time[1] = (t[4] + t[5] + t[6] + t[7]) / 4;
    double vv = length_ / (abs(arrive_time[0] - arrive_time[1]) * 1e-9);
    return ENERGY_OF_MOUN * vv * vv;
}

#endif