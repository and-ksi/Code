#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define PI (acos(-1))
#define CHANNEL_NUM (8)
#define MMAP_SIZE (8192)

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
pthread_t ptd[CHANNEL_NUM];
int channel_id;
bool interrupt = 0;
char *data;
int data_length;
char subdata[1024];

double sinfunc(double x, int a){
    double f = 5*sin(PI*2e6*x + a);
    return f;
}

void mmap_open(){
    int fd = shm_open("shm01", O_CREAT | O_RDWR, 0777);
    if(fd < 0){
        printf("Share mem open failed!");
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

int data_generate(int id){
    struct timeval time0, starttime;
    double time;
    int count = 0;

    gettimeofday(&starttime, 0);
    while(1){
        gettimeofday(&time0, 0);
        time = time0.tv_sec - starttime.tv_sec + (time0.tv_usec - starttime.tv_usec)*1e-6;
        while(sinfunc(time, 0) >= 1){
            sprintf(subdata + count * 16, "%4d%12.11f", id, sinfunc(time, 0));
            count++;
        }
    }
    return (count + 1) / 2;
}

void channel_generate(){
    int id = channel_id;
    channel_id++;
    if(channel_id < CHANNEL_NUM){
        pthread_create(&ptd[channel_id], 0, (void *(*)(void *))channel_generate, NULL);
    }

    sprintf(frame_head.channle_id, "%d", id);
    memcpy(frame_head.error, "NULL", 4);
    memcpy(frame_head.Ftype, "00", 2);
    data_length = 0;


    
    
}

int main(){
    memset(&board_head, '\0', sizeof(board_head));
    memset(&frame_head, '\0', sizeof(frame_head));

    memcpy(board_head.board_addr, "99999999", 8);
    channel_id = 0;







    return 0;
}