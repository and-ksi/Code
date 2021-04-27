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
#define PACK_SIZE (4 * 1024)
#define CLIENT_NUM (7)

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

BOARD_HEAD board_head;
FRAME_HEAD frame_head;
int channel_id;
int *signal; //4:stop    0:write     1:wait for read
char *data;
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

void mmap_open()
{
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

    if (k >= 0 && k < CPU_CORE - 1)
    { //k为-1时不使用核心亲和属性
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

void data_send0(char *mes, int *fd, int id)
{
    int ret;

    ret = send(*fd, mes, PACK_SIZE, 0);
    if (ret < 0)
    {
        printf("第%d次发送 : 对%d客户端 : Send failed!\n", count, id);
        exit(1);
    }else{
        //printf("第%d次发送 : 对%d客户端 : Send success!\n", count, id);
        memset(mes, '0', PACK_SIZE);
    }
}

void *data_send()
{
    int id = ptd_id;
    printf("Id为%d的发送线程已创建完毕。\n", id);
    ptd_alarm = 0;

    int ret;
    count = 0;

    while (global_alarm == 0)
    {
        while (send_alarm == 0) ;
        for (int i = 0; i < CLIENT_NUM; i++)
        {
            data_send0(pack_send[i], &acfd[i], i);
            count++;
        }
        send_alarm = 0;
    }
    return NULL;
}

//socket和线程创建函数
void socket_create()
{
    int ret;
    int on = 1;
    int i;
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
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
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

    for (i = 0; i < CLIENT_NUM; i++)
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

    char channel_id[9] = {'\0'};
    int cpy_length;
    int ret;
    int frame_length;
    char zero_buf[32];
    int part;
    char buf[32];

    memset(&pack_send, '0', sizeof(pack_send));
    memset(zero_buf, '0', 32); //加上尺寸条件
    memset(buf, '\0', 32);

    while (*signal == 0 || send_alarm == 1)
        ;

    for (int i = 0; i < CLIENT_NUM; i++)
    {
        memcpy(pack_send[i], data, 32);
        pack_length[i] = 32;
    }
    cpy_length = 32; //将board——head

    while (global_alarm == 0)
    {
        while (*signal == 0 || send_alarm == 1)
            ;
        while (cpy_length < MMAP_SIZE)
        {
            if (!memcmp(zero_buf, data + cpy_length, 32))
            {
                break;
            }
            memcpy(channel_id, data + cpy_length, 8);
            ret = atoi(channel_id);

            if (ret < CHANNEL_NUM && ret >= 0)
            {
                part = ret % CLIENT_NUM;
                memcpy(buf, data + cpy_length + 16, 16);
                ret = atoi(buf);
                memset(buf, '\0', 32);

                memcpy(pack_send[part] + pack_length[part], data + cpy_length, 32 * 3 + ret);
                pack_length[part] += ret + 32 * 3;
                cpy_length += 32 * 3 + ret;
            }
            else
            {
                printf("Data read error! Channel:%d\n", ret);
                exit(1);
            }
            //这里需要做数据不连续的处理
            //应该增加一种条件，即pData没有存储满时读取已存储部分;已解决
        }
        memset(data, '0', MMAP_SIZE);
        *signal = 0;
        send_alarm = 1;
        for (int i = 0; i < CLIENT_NUM; i++)
        {
            pack_length[i] = 0;
        }
        cpy_length = 0;
    }
    return NULL;
}

int main()
{
    char sig;
    pthread_t part_ptd, send_ptd;

    mmap_open();
    signal_mmap_open();

    socket_create();
    ptd_create(&send_ptd, 1, (void *(*))data_send);
    ptd_create(&part_ptd, 2, (void *(*))data_part);

    while (sig != '0')
    {
        sig = getchar();
        switch (sig)
        {
        case '1':
            while (send_alarm == 1) //回头再改
                ;
            sig = '0';
            break;
        case '2':
            printf("第%d次发送 : Send success!\n", count);
        }
    }
    global_alarm = 1;
    close(data_fd);
    close(signal_fd);
    for (int i = 0; i < CLIENT_NUM; i++)
    {
        close(acfd[i]);
    }
    close(socket_fd);
    return 0;
}