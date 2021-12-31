#include <arpa/inet.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define CHANNEL_NUM (8)
#define MMAP_SIZE (8 * 1024)
#define CPU_CORE (4) //CPU核心数量,使用顺序从数值最大的核心开始分配,留下第一个核心不分配
#define PACK_SIZE (2 * 1024)
#define CLIENT_NUM (1)
#define CHANNLE_NUM (8)

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
int socket_fd;
int port = 10000;
int acfd[10];
char pack_send[CLIENT_NUM][PACK_SIZE]; //每个客户端对应的待发送数据包
int ptd_alarm = 0, send_alarm = 0, global_alarm = 0;
int ptd_id, count;
int pack_length[CLIENT_NUM] = {0};
struct sockaddr_in clientaddr[10] = {0};
pthread_t ptd[10];

void mmap_open(){
    data_fd = shm_open("shm01", O_CREAT | O_RDWR, 0777);
    if (data_fd < 0)
    {
        printf("Share memery open failed!\n");
        exit(1);
    }
    ftruncate(data_fd, MMAP_SIZE);
    data = (char *)mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, data_fd, 0);
    if (!data)
    {
        printf("Mmap failed!\n");
        close(data_fd);
        exit(1);
    }
}

void signal_mmap_open()
{
    signal_fd = shm_open("shm02", O_CREAT | O_RDWR, 0777);
    if (signal_fd < 0)
    {
        printf("Share memery open failed!\n");
        exit(1);
    }
    ftruncate(signal_fd, 4);
    signal = (int *)mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, signal_fd, 0);
    if (!signal)
    {
        printf("Signal mmap failed!\n");
        exit(1);
    }
}

//根据CPU创建和分配线程
void ptd_create(pthread_t *arg, int k, void *functionbody)
{
    int ret;

    pthread_attr_t attr;

    ret = pthread_attr_init(&attr); //初始化线程属性变量,成功返回0,失败-1
    if (ret < 0)
    {
        perror("Init attr fail\n");
        exit(1);
    }

    if(k != -1){//k为-1时不使用核心亲和属性
        cpu_set_t cpusetinfo;
        CPU_ZERO(&cpusetinfo);
        CPU_SET((CPU_CORE - 1 - k), &cpusetinfo); //将core1加入到cpu集中,同理可以将其他的cpu加入

        ret = pthread_attr_setaffinity_np(&attr, sizeof(cpusetinfo), &cpusetinfo);
        if (ret < 0)
        {
            perror("Core set fail\n");
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
        perror("Detached fail!\n");
        exit(1);
    }

    pthread_create(arg, &attr, (void *(*)(void *))functionbody, NULL);
    //printf("Id为%d的线程已创建完毕。", *arg);

    pthread_attr_destroy(&attr); //销除线程属性
}

void *data_send(){
    int id = ptd_id;
    printf("Id为%d的发送线程已创建完毕。\n", id);
    ptd_alarm = 0;

    char buf[8] = {'\0'};
    int ret;
    int part;

    while (!global_alarm)
    {
        while (send_alarm == 0);
        for (int i = 0; (part = id + CPU_CORE * i) < CLIENT_NUM; i++)
        {
            ret = send(acfd[part], pack_send[part], PACK_SIZE, 0);
            if (ret < 0)
            {
                printf("第%d次发送 , 线程id: %d : Send failed!\n", count, id);
                exit(1);
            }
            memset(&pack_send[part], '0', PACK_SIZE);
        }
        send_alarm = 0;
    }
    return NULL;
}

//socket和线程创建函数
void socket_ptd_create()
{
    int ret;
    socklen_t len;

    struct sockaddr_in localaddr = {0};
    localaddr.sin_family = AF_INET;
    localaddr.sin_port = htons(port);
    localaddr.sin_addr.s_addr = INADDR_ANY;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        printf("Socket fail!\n");
        exit(1);
    }
    ret = bind(socket_fd, (struct sockaddr *)&localaddr, sizeof(localaddr));
    if (ret < 0)
    {
        printf("Bind fail!\n");
        exit(1);
    }
    ret = listen(socket_fd, 10);
    if (ret < 0)
    {
        printf("Listen fail!\n");
        exit(1);
    }

    for (int i = 0; i < CPU_CORE - 1; i++)
    {
        printf("创建第%d个线程...\n", i);
        ptd_alarm = 1;
        ptd_id = i;
        ptd_create(&ptd[i], ptd_id, (void *(*))data_send);
        while (ptd_alarm == 1);
    }

    for (int i = 0; i < CLIENT_NUM; i++)
    {
        printf("等待客户端连接...\n");
        acfd[i] = accept(socket_fd, (struct sockaddr *)&clientaddr[i], &len);
        if (acfd[i] < 0)
        {
            perror("Accept fail\n");
            exit(1);
        }
        printf("第%d个客户端已连接!\n", i); //希望能够显示连接的客户端地址
    }
}

//数据分发
void *data_part()
{
    printf("Part 线程已创建!\n");
    ptd_alarm = 0;

    char channle_id[9] = {'\0'};
    int cpy_length;
    int ret;
    int frame_length;
    char zero_buf[32];
    int part;

    memset(&pack_send, '0', sizeof(pack_send));
    memset(zero_buf, '0', 32);

    while (*signal == 0 || send_alarm == 1);

    for (int i = 0; i < CLIENT_NUM; i++)
    {
        memcpy(pack_send[i] + 32, data, 32);
        pack_length[i] = 32;
    }
    cpy_length = 32;

    while (!global_alarm)
    {
        while (*signal == 0 || send_alarm == 1);
        while (cpy_length < MMAP_SIZE)
        {
            memcpy(channle_id, data + cpy_length, 8);
            ret = atoi(channle_id);
            if (ret == 0)
            {
                if (!memcmp(zero_buf, data + cpy_length, 32))
                {
                   break;
                }
            }
            if (ret < CHANNLE_NUM)
            {
                memcpy(buf, data + cpy_length + 16, 16);
                ret = atoi(buf);
                part = ret % CLIENT_NUM;

                memcpy(pack_send[part] + pack_length[part], data + cpy_length, 32 * 3 + ret);
                pack_length[part] += ret + 32 * 3;
                cpy_length =+ 32 * 3 + ret;
            }
            //这里需要做数据不连续的处理
            //应该增加一种条件，即pData没有存储满时读取已存储部分
        }
        memset(data, '0', MMAP_SIZE);
        *signal = 0;
        send_alarm = 1;
        cpy_length = 0;
    }
    return NULL;
}

int main(){
    char sig;
    pthread_t part_ptd;

    mmap_open();
    signal_mmap_open();

    socket_ptd_create();
    ptd_create(&part_ptd, -1, (void *(*))data_part);

    while(sig != '0'){
        sig = getchar();
        switch (sig)
        {
        case '1':
            while(send_alarm == 1);
            sig = '0';
            break;
        }
    global_alarm = 1;
    close(data_fd);
    close(signal_fd);
    for(int i = 0; i < CLIENT_NUM; i++){
        close(acfd[i]);
    }
    close(socket_fd);
    return 0;
    }
}