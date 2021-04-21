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
#define MMAP_SIZE (8 * 1024)
#define CPU_CORE (4) //CPU核心数量,使用顺序从数值最大的核心开始分配,留下第一个核心不分配
#define PACK_SIZE (8 * 1024)
#define CLIENT_NUM (7)
#define CHANNLE_NUM (7)

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
int *signal; //4:stop    0:write     1:wait for read
char *data;
char buf[32];
int data_fd, signal_fd;
int port = 10000;
int acfd[10];
char pack_send[CLIENT_NUM][PACK_SIZE]; //每个客户端对应的待发送数据包

void mmap_open(){
    data_fd = shm_open("shm01", O_CREAT | O_RDWR, 0777);
    if (data_fd < 0)
    {
        printf("Share memery open failed!");
        exit(1);
    }
    ftruncate(data_fd, MMAP_SIZE);
    data = (char *)mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, data_fd, 0);
    if (!data)
    {
        printf("Mmap failed!");
        close(data_fd);
        exit(1);
    }
}

void signal_mmap_open()
{
    signal_fd = shm_open("shm02", O_CREAT | O_RDWR, 0777);
    if (signal_fd < 0)
    {
        printf("Share memery open failed!");
        exit(1);
    }
    ftruncate(signal_fd, 4);
    signal = (int *)mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, signal_fd, 0);
    if (!signal)
    {
        printf("Signal mmap failed!");
        exit(1);
    }
}

//根据CPU创建和分配线程
void ptd_create(pthread_t *arg, void *functionbody)
{
    int ret;

    cpu_set_t cpusetinfo;
    CPU_ZERO(&cpusetinfo);
    CPU_SET((CPU_CORE - 1 - *arg), &cpusetinfo); //将core1加入到cpu集中,同理可以将其他的cpu加入

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
    ret = pthread_attr_setaffinity_np(&attr, sizeof(cpusetinfo), &cpusetinfo);
    if (ret < 0)
    {
        perror("Core set fail");
        exit(1);
    }

    pthread_create(arg, &attr, (void *(*)(void *))functionbody, NULL);
    //printf("Id为%d的线程已创建完毕。", *arg);

    pthread_attr_destroy(&attr); //销除线程属性
}

void *data_send(){
    
}