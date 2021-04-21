#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PI (acos(-1))
#define CHANNEL_NUM (8)
#define MMAP_SIZE (8192)
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
int channel_id;
int *signal;
char *data;
int data_length, sub_length;
char subdata[SUBDATA_SIZE];
long int time0, starttime, time1;
char buf[32];

double sinfunc(double x, int a){
    double f = 5*sin(PI*2e6*x + a);
    return f;
}

void mmap_open(){
    int fd = shm_open("shm01", O_CREAT | O_RDWR, 0777);
    if(fd < 0){
        printf("Share memery open failed!");
        exit(1);
    }
    ftruncate(fd, MMAP_SIZE);
    data = (char *)mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(!data){
        printf("Mmap failed!");
        close(fd);
        exit(1);
    }
    memset(data, '0', MMAP_SIZE);
}

void signal_mmap_open(){
    int fd = shm_open("shm02", O_CREAT | O_RDWR, 0777);
    if (fd < 0)
    {
        printf("Share memery open failed!");
        exit(1);
    }
    ftruncate(fd, 4);
    signal = (int *)mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    *signal = 0;
}

void subdata_generate(int id){
    double time, ret;

    memset(subdata, '0', SUBDATA_SIZE);
    time = time0 * 1e-6;

    sprintf(buf, "%ld", time0);
    memcpy(frame_head.timestamp, buf, strlen(buf));
    memset(buf, '0', 32);
    while((ret = sinfunc(time, id)) >= 1){
        sprintf(buf, "%12.11f", ret);
        memcpy(subdata + sub_length + 4, buf, 12);
        memset(buf, '0', 32);
        sub_length += 16;
        time0++;
    }
    if(time0 % 2 == 1){
        sub_length += 16;
    }
    sprintf(buf, "%016d", sub_length);
    memcpy(frame_head.length, buf, 16);
    memset(buf, '0', 32);
    sub_length += 32 * 3;
}

void channel_generate(){
    memset(&board_head, '0', sizeof(board_head));
    memset(&frame_head, '0', sizeof(frame_head));

    memcpy(board_head.board_addr, "99999999", 8);
    channel_id = 0;
    data_length = 32;
    time0 = 0;

    while(*signal != 4){
        if (channel_id = CHANNEL_NUM)
            channel_id = 0;
        memset(buf, '0', 32);
        memset(frame_head.error, '1', 6);
        memset(frame_head.Ftype, '2', 2);
        sprintf(buf, "%08d", channel_id);
        memcpy(frame_head.channle_id, buf, 8);
        memset(buf, '0', 32);
        sub_length = 0;

        subdata_generate(channel_id);
        if((MMAP_SIZE - data_length) <= sub_length){
            
        }

        memcpy(data + data_length, &frame_head, 32 * 3);
        memcpy(data + data_length, subdata, sub_length);

        channel_id++;
        memset(&frame_head, '0', sizeof(frame_head));
    }
}

int main(){
    







    return 0;
}