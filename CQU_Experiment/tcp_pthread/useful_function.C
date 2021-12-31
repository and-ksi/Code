#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define CHANNEL_NUM (8)

typedef struct board_head
{
    char board_type[8];
    char board_addr[8];
    char Ftype[2];
    char Error[14];
} BOARD_HEAD;

typedef struct frame_head
{
    char channel_id[8];
    char error[6];
    char Ftype[2];
    char length[16];
    //char timestamp_H[32];
    //char timestamp_L[32];
    char timestamp[64];
} FRAME_HEAD;

//根据is不同的返回值
int64_t struct_head_read(void *body, char is)
{
    if (!body || is == 0)
    {
        printf("Void pointer or error arg!");
        return -1;
    }
    char outbuf[4][32] = {'\0'};

    switch (is)
    {
    case 'b'://输出board info
        BOARD_HEAD *bb = (BOARD_HEAD *)body;
        memcpy(outbuf[0], bb->board_addr, 8);
        memcpy(outbuf[1], bb->board_type, 8);
        memcpy(outbuf[2], bb->Error, 14);
        memcpy(outbuf[3], bb->Ftype, 2);
        printf("**********BOARD INFO**********\n");
        printf("Board type:%s \nBoard addr:%s\nBoard Ftype:%s\nBoard Error:%s\n\n", outbuf[1],
               outbuf[0], outbuf[3], outbuf[2]);
        return 0;
        break;
    
    case 'f'://输出frame info
        FRAME_HEAD *cc = (FRAME_HEAD *)body;
        memcpy(outbuf[0], cc->channel_id, 8);
        memcpy(outbuf[1], cc->error, 6);
        memcpy(outbuf[2], cc->Ftype, 2);
        memcpy(outbuf[3], cc->length, 16);
        printf("**********FRAME INFO**********\n");
        printf("channel_id: %s\nError: %s\nFtype: %s\nLength: %s\n\n",
               outbuf[0], outbuf[1], outbuf[2], outbuf[3]);
        return atoi(outbuf[3]);
        break;

    case 'l'://返回adc data 的length（64位整型）
        FRAME_HEAD *cc = (FRAME_HEAD *)body;
        memcpy(outbuf[3], cc->length, 16);
        return atoi(outbuf[3]);
        break;

    case 'c'://返回adc data's channel_id(int64_)
        FRAME_HEAD *cc = (FRAME_HEAD *)body;
        memcpy(outbuf[0], cc->channel_id, 8);
        return atoi(outbuf[0]);
        break;

    case 't'://back adc data's timestamp(int64_)
        FRAME_HEAD *cc = (FRAME_HEAD *)body;
        char timest[65] = {'\0'};
        memcpy(timest, cc->timestamp, 64);
        return atol(timest);
        break;
    }
}

//读取数据
//主要将全为0/1的数据转为正负5V的double;同时也可以输出数据前4位的channel_id
double data_read_func(char *inp, char is){
    switch (is)
    {
    case 'v':
        double m = 10. / 4096.; //2^12 = 4096
        int sum = 0;
        for (int i = 15; i > 3; i--)
        {
            if (*(inp + i) == '1')
            {
                sum += ldexp(1, 15 - i);
            }
        }
        return (double)sum * m;
        break;
        
    case 'c':
        char no[5] = {'\0'};
        memcpy(no, inp, 4);
        return (double)atoi(no);
        break;
    }
}

//输入数据指针,frame_head指针,恒比定时
typedef struct cfd_ana{
    int64_t timestamp_data;//frame_head.timestamp
    int time;//触发信号起始时间,即第i次探测
    double value;//the value of f(t)
}CFD_ANA;
CFD_ANA cfd_data[CHANNEL_NUM][1024];//每个通道1024个
int cfdfunc(void *head_pd){
    double p = 1.05;
    int cfdoffset = 50;
    int cfdthresh = -60;

    FRAME_HEAD data_head;
    memcpy(&data_head, head_pd, 32*3);
    int data_num = struct_head_read(&data_head, 'l') / 16;
    double data_ana[1024];//这个数组根据需要改变大小
    int tip = 0, k = 0, len = 32 * 3;
    int channel = struct_head_read(&data_head, 'c');

    for(int i = 0; i < data_num; i++){
        data_ana[i] = data_read_func((char *)(head_pd + len), 'v');
        len += 16;
        if(i > cfdoffset){
            data_ana[i] += -p * data_ana[i - 50];
            if(data_ana[i] <= cfdthresh && tip == 0){
                tip = 1;
                cfd_data[channel][k].time = i;
                cfd_data[channel][k].timestamp_data = struct_head_read(head_pd, 't');
                cfd_data[channel][k].value = data_ana[i];
                k++;
            }else if(data_ana[i] > cfdthresh){
                tip = 0;
            }
        }
    }
    return k;
}

//speed test pthread
#include <sys/time.h>
#include <stdio.h>
void *speed_test(){
    int cnt;//注释掉,使用global identifier代替
    struct timeval systime;
    long time0, time1, time2 = 0, cnt1 = 0;
    double speed0, speed1;
    int size_of_every_time = 1024;//1024kB
    gettimeofday(&systime, 0);
    time0 = systime.tv_sec * 1e6 + systime.tv_usec;
    while(1){
        gettimeofday(&systime, 0);
        time1 = systime.tv_sec * 1e6 + systime.tv_usec - time0;
        while(time1 - time2 > 1){//calculate every second
            //speed in one seconds
            speed0 = (double)(cnt - cnt1) / (double)((time1 - time2) * 1e6 * size_of_every_time);
            //total average speed
            speed1 = (double)(cnt) / (double)(time1 * 1e6 * size_of_every_time);
            printf("Average speed: %lfMB/s, second speed: %lfMB/s\n", 
             speed0, speed1);
            time2 = time1;
            cnt1 = cnt;
        }
    }
}

//base on the cfd_data function, paint the point to the canvas