#include <arpa/inet.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PACK_SIZE (4 * 1024)

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

char IP[] = "192.168.3.1";
char pack_recved[PACK_SIZE];
int port = 10000;
int recv_alarm, ana_alarm, ptd_alarm, global_alarm;
int recv_count;
int socket_fd;
int connect_fd;
int read_length;
int cpy_length;
char data_pack[2][PACK_SIZE];
int channel[2];

void *pack_recv()
{
    printf("Socket接收线程已创建!\n");
    ptd_alarm = 1;

    int ret;
    int on = 1;
    recv_count = 0;

    struct sockaddr_in serveraddr = {0};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_pton(AF_INET, IP, &serveraddr.sin_addr.s_addr);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        printf("Socket fail!\n");
        exit(1);
    }
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    connect_fd = connect(socket_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (connect_fd < 0)
    {
        printf("Connect fail!\n");
        close(socket_fd);
        exit(1);
    }
    memset(&pack_recved, '0', PACK_SIZE);

    while (global_alarm == 0)
    {
        while (recv_alarm == 1);
        ret = recv(socket_fd, pack_recved, PACK_SIZE, 0);
        if (ret < 0)
        {
            printf("Recv fail!\n");
            break;
        }
        recv_count++;
        recv_alarm = 1;
    }
    return NULL;
}

void *data_analys()
{
    printf("Ana 线程已创建!\n");
    ptd_alarm = 1;

    long int length;
    long int ret;
    char zero_buf[32];
    char buf[5][32];
    char channel_id[9];
    char data_length[17];
    int mark = 0;

    memset(data_length, '\0', 17);
    memset(channel_id, '\0', 9);
    memset(buf, '\0', sizeof(buf));
    memset(zero_buf, '0', 32);
    channel[0] = channel[1] = -1;//不相同，自用，相同，转存另一个
    while (recv_alarm == 0)
        ;

    memcpy(buf[0], pack_recved, 8);
    memcpy(buf[1], pack_recved + 8, 8);
    memcpy(buf[2], pack_recved, 2);
    memcpy(buf[3], pack_recved + 18, 14);
    printf("Board INFO: \nBoard type:%s \nBoard addr:%s\nBoard Ftype:%s\nBoard Error:%s\n",
           buf[0], buf[1], buf[2], buf[3]);
    memset(buf, '\0', sizeof(buf));
    length = 32;

    memcpy(channel_id, pack_recved + length, 8);
    channel[1] = atoi(channel_id);

    while (global_alarm == 0)
    {
        while (recv_alarm == 0)
            ;
        while (length < PACK_SIZE)
        {
            if (!memcmp(zero_buf, pack_recved + length, 32))
            {
                break;
            }
            memcpy(channel_id, pack_recved + length, 8);
            channel[0] = atoi(channel_id);

            if(channel[0] == channel[1]){
                mark = 1;
            }
            if (channel[mark] >= 0)
            {
                memcpy(buf[0], pack_recved + length + 8, 6);
                memcpy(buf[1], pack_recved + length + 14, 2);
                memcpy(data_length, pack_recved + length + 16, 16);
                ret = atoi(data_length);
                length += ret + 3 * 32;
                printf("ADC INFO:\nchannel_id: %s\nError: %s\nFtype: %s\nLength: %ld\n",
                       channel_id, buf[0], buf[1], ret);
                printf("debug info: length: %ld, channel: %d\n", length, channel[mark]);
                memset(buf, '\0', sizeof(buf));
            }
            mark = 0;
        }
        length = 0;
        memset(pack_recved, '0', PACK_SIZE);
        recv_alarm = 0;
    }
    return NULL;
}

//根据CPU创建和分配线程
void ptd_create(pthread_t *arg, int k, void *functionbody)
{
    int ret;

    pthread_attr_t attr;

    ret = pthread_attr_init(&attr); //初始化线程属性变量,成功返回0,失败-1
    if (ret < 0)
    {
        perror("Init attr fail");
        exit(1);
    }

    /* if (k != -1)
    { //k为-1时不使用核心亲和属性
        cpu_set_t cpusetinfo;
        CPU_ZERO(&cpusetinfo);
        CPU_SET((CPU_CORE - 1 - k), &cpusetinfo); //将core1加入到cpu集中,同理可以将其他的cpu加入

        ret = pthread_attr_setaffinity_np(&attr, sizeof(cpusetinfo), &cpusetinfo);
        if (ret < 0)
        {
            perror("Core set fail");
            exit(1);
        }
    } */

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

int main()
{
    char sig;

    recv_alarm = 0;
    ana_alarm = 0;
    global_alarm = 0;

    pthread_t recv_ptd, ana_ptd;
    ptd_alarm = 0;
    ptd_create(&recv_ptd, 0, (void *(*))pack_recv);
    while (ptd_alarm == 0)
        ;
    ptd_create(&ana_ptd, 0, (void *(*))data_analys);
    while (ptd_alarm == 0)
        ;

    while (sig != '0')
    {
        sig = getchar();
        switch (sig)
        {
        case '1':
            while (recv_alarm == 0)
                ;
            while (recv_alarm == 1)
                ;
            sig = '0';
            break;
        }
    }
    global_alarm = 1;
    close(connect_fd);
    close(socket_fd);
    return 0;
}