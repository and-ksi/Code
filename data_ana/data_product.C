#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PI (3.1415956)
#define CHANNEL_NUM (8)
#define MMAP_SIZE (8 * 1024)
#define SUBDATA_SIZE (1024)

typedef struct board_head
{
    char board_type[8];
    char board_addr[8];
    char Ftype[2];
    char Error[14];
} BOARD_HEAD;

typedef struct frame_head
{
    char channle_id[8];
    char error[6];
    char Ftype[2];
    char length[16];
    //char timestamp_H[32];
    //char timestamp_L[32];
    char timestamp[64];
} FRAME_HEAD;

BOARD_HEAD board_head;
FRAME_HEAD frame_head;
int channel;
int *signal;//4:stop    0:write     1:wait for read
char *data;
int data_length, sub_length;
char subdata[SUBDATA_SIZE];
long int time0, starttime, time1;
char buf[32];
int data_fd, signal_fd;

double sinfunc(double x, int a){
    double f;
    f = 5 * sin(PI * 2e6 * x + (double)a);
    return f;
}

void mmap_open(){
    data_fd = shm_open("shm01", O_CREAT | O_RDWR, 0777);
    if(data_fd < 0){
        printf("Share memery open failed!\n");
        exit(1);
    }
    ftruncate(data_fd, MMAP_SIZE);
    data = (char *)mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, data_fd, 0);
    if(!data){
        printf("Mmap failed!\n");
        close(data_fd);
        exit(1);
    }
    memset(data, '0', MMAP_SIZE);
}

void signal_mmap_open(){
    signal_fd = shm_open("shm02", O_CREAT | O_RDWR, 0777);
    if (signal_fd < 0)
    {
        printf("Share memery open failed!\n");
        exit(1);
    }
    ftruncate(signal_fd, 4);
    signal = (int *)mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, signal_fd, 0);
    if(!signal){
        printf("Signal mmap failed!\n");
        exit(1);
    }
    *signal = 0;
}

void subdata_generate(int id){
    double time, ret;

    memset(subdata, '0', SUBDATA_SIZE);
    time = (double)time0 * 0.0000001;

    while((ret = sinfunc(time, id)) < 1){
        time0++;
        time = (double)time0 * 0.0000001;
    }
    sprintf(buf, "%09d", time0);//64位的时间戳这里仅使用十位
    memcpy(frame_head.timestamp, buf, strlen(buf) + 1);
    memset(buf, '\0', 32);
    while((ret = sinfunc(time, id)) >= 1){
        sprintf(buf, "%12.11f", ret);
        memcpy(subdata + sub_length + 4, buf, 12);
        memset(buf, '\0', 32);
        sub_length += 16;
        time0++;
        time = (double)time0 * 0.0000001;
    }
    if(sub_length % 32 != 0){
        sub_length += 16;
    }
    sprintf(buf, "%016d", sub_length);
    memcpy(frame_head.length, buf, 16);
    memset(buf, '\0', 32);
}

void *channel_generate(){
    pthread_detach(pthread_self());
    memset(&board_head, '0', sizeof(board_head));
    memset(&frame_head, '0', sizeof(frame_head));

    memcpy(board_head.board_addr, "99999999", 8);
    memset(board_head.board_type, '1', 8);
    memset(board_head.Error, '2', 14);
    memset(board_head.Ftype, '8', 2);
    memcpy(data, &board_head, 32);
    channel = 0;
    data_length = 32;
    time0 = 0;
    char exp[MMAP_SIZE];

    while(*signal != 4){
        if (channel == CHANNEL_NUM){
            channel = 0;
            }
        memset(buf, '\0', 32);
        memset(frame_head.error, '1', 6);
        memset(frame_head.Ftype, '2', 2);
        sprintf(buf, "%08d", channel);
        memcpy(frame_head.channle_id, buf, 8);
        memset(buf, '\0', 32);
        sub_length = 0;

        subdata_generate(channel);
        if((MMAP_SIZE - data_length) < (sub_length + 3 * 32)){
            *signal = 1;
            memcpy(exp, data, MMAP_SIZE);//debug
            printf("本次发送数据长度：%d.\n", data_length);
            while(*signal == 1);
            data_length = 0;
        }

        memcpy(data + data_length, &frame_head, 32 * 3);
        memcpy(data + data_length + 32 * 3, subdata, sub_length);
        memset(subdata, '0', SUBDATA_SIZE);
        data_length += sub_length + 32 * 3;

        channel++;
        memset(&frame_head, '0', sizeof(frame_head));
    }
    return NULL;
}

int main(){
    char sig;

    mmap_open();
    signal_mmap_open();
    
    pthread_t ptd;
    pthread_create(&ptd, 0, (void *(*)(void *))channel_generate, 0);

    while(sig != '0'){
        sig = getchar();
        switch (sig)
        {
        case '1':
            while(*signal == 1);
            close(data_fd);
            close(signal_fd);
            sig = '0';
            break;
        }
    }
    close(data_fd);
    close(signal_fd);
    return 0;
}