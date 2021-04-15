#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <memory.h>
#include <netinet/in.h>
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

#define PACK_SIZE (8 * 1024)

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
    char timestamp_H[32];
    char timestamp_L[32];
} FRAME_HEAD;

char IP[32] = "192.168.3.1";
char pack_recved[PACK_SIZE];
int port = 10000;
int recv_alarm, recv_count, ana_alarm, ptd_alarm;
int read_length;
int cpy_length;
char data_pack[2][PACK_SIZE];

void *pack_recv(){
    ptd_alarm = 1;

    int socket_fd;
    int connect_fd;
    int ret;
    recv_count = 0;

    struct sockaddr_in serveraddr = {0};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = port;
    inet_pton(AF_INET, IP, &serveraddr.sin_addr.s_addr);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        printf("Socket fail!\n");
        exit(1);
    }
    connect_fd = connect(socket_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (connect_fd < 0)
    {
        printf("Connect fail!\n");
        exit(1);
    }
    memset(&pack_recved, '\0', PACK_SIZE);
    recv_alarm = 0;
    ana_alarm = 0;

    while(1){
        while(recv_alarm == 1);
        ret = recv(socket_fd, pack_recved, PACK_SIZE, 0);
        if (ret < 0)
        {
            printf("Recv fail!\n");
            break;
        }
        recv_count++;
        recv_alarm = 1;
    }
    exit(1);
}

void *data_read(){
    ptd_alarm = 1;

    int ret;
    char buf[8] = {0};
    int part = 0, channle_id[2];
    cpy_length = 0;
    read_length = 0;
    
    while(1){
        while(recv_alarm == 0 || ana_alarm == 1);
        memcpy(buf, pack_recved, 8);
        ret = atoi(buf);
        if(ret != recv_count){
            printf("recv_count: %d :Data recved ERROR!", recv_count);
            exit(1);
        }
        while(cpy_length < PACK_SIZE){
            memcpy(buf, pack_recved + cpy_length + 32, 8);
            channle_id[1] = atoi(buf);
            if(channle_id[0] != channle_id[1]){
                part++;
            }
            memcpy(buf, pack_recved + cpy_length + 8, 16);
            ret = atoi(buf);
            cpy_length = cpy_length + 32;
            memcpy(data_pack[part] + read_length, pack_recved + cpy_length, ret);
            cpy_length = cpy_length + ret;
            read_length = read_length + ret;
            part = 0;
        }
        cpy_length = 0;
        read_length = 0;
        recv_alarm = 0;
    }
    return NULL;
}

void *data_analys(){
    ptd_alarm = 1;
    BOARD_HEAD board_head;
    FRAME_HEAD frame_head;
    int length;
    char void_mark[8] = {'\0'};
    while(ana_alarm == 0);

    memcpy(board_head.board_type, data_pack, 8);
    memcpy(board_head.board_addr, data_pack + 8, 8);
    memcpy(board_head.Ftype, data_pack, 2);
    memcpy(board_head.Error, data_pack + 18, 14);
    printf("Board INFO: Board type:%s \nBoard addr:%s\nBoard Ftype:%s\nBoard Error:%s", 
     board_head.board_type, board_head.board_addr, board_head.Ftype, board_head.Error);
    length = 32;

    while (1)
    {
        while(ana_alarm == 0);
        while(memcmp(data_pack + length, void_mark, 8)){
            memcpy(frame_head.channle_id, data_pack + length, 8);
            memcpy(frame_head.error, data_pack + length + length + 8, 6);
            memcpy(frame_head.Ftype, data_pack + length + 14, 2);
            memcpy(frame_head.length, data_pack + length + 16, 16);
            length = atoi(frame_head.length) + length + 32*3;
            printf("ADC INFO: Channle id: %s    Error: %s    Ftype: %s     Length: %s", 
                frame_head.channle_id, frame_head.error, frame_head.Ftype, frame_head.length);
            memset(&frame_head, '\0', sizeof(frame_head));
        }
        length = 0;
        ana_alarm = 0;
    }
    return NULL;
}

void ptd_create(pthread_t *arg, void *functionbody)
{
    int ret;

    pthread_attr_t attr;

    ret = pthread_attr_init(&attr); //初始化线程属性变量,成功返回0,失败-1
    if (ret < 0)
    {
        perror("Init attr fail");
        exit(1);
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

int main(){
    pthread_t pack_recv_ptd;
    pthread_t data_read_ptd;
    pthread_t data_analys_ptd;
    ptd_alarm = 0;
    ptd_create(&pack_recv_ptd, (void *(*))pack_recv);
    while(ptd_alarm == 0);
    ptd_create(&data_read_ptd, (void *(*))data_read);
    while (ptd_alarm == 0);
    ptd_create(&data_analys_ptd, (void *(*))data_analys);
    while (ptd_alarm == 0);
    return 0;
}