#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>

#define PI (acos(-1))
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

float sinfun(float x, int a){
    float f = 5*sin(PI*2e6*x + a);
    return f;
}

void *channel_generate(){
    pthread_detach(pthread_self());
    int id = channel_id;
    channel_id++;
    if(channel_id < CHANNEL_NUM){
        pthread_create(&ptd[channel_id], 0, (void *(*)(void *))channel_generate, NULL);
    }

    sprintf(frame_head.channle_id, "%d", id);
    memcpy(frame_head.error, "NULL", 4);
    memcpy(frame_head.Ftype, "00", 2);

    while (interrupt == 0)
    {
        
    }
    
    
}

void data_generate(){
    memset(&board_head, '\0', sizeof(board_head));
    memset(&frame_head, '\0', sizeof(frame_head));

    memcpy(board_head.board_addr, "99999999", 8);
    channel_id = 0;

    pthread_create(&ptd[channel_id], 0, (void *(*)(void *))channel_generate, NULL);

}













int main(){

    return 0;
}