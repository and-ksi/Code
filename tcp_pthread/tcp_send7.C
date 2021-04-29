#include "recv_ana1.h"

int work;

int c2h_fd;
int h2c_fd;
int control_fd;
void *control_base;
int interrupt_fd;
int read_end_addr;

int pData_0[RX_SIZE / 4];
int pData_1[RX_SIZE / 4];

static sem_t int_sem_rx;
static sem_t int_sem_rx0;

int acfd[CLIENT_NUM];
int port = 10000;

int pack_send[8][RX_SIZE / CLIENT_NUM];

int socket_fd;
struct sockaddr_in clientaddr[CLIENT_NUM];

/*中断处理进程*/
void *event_process()
{
    interrupt_fd = open_event("/dev/xdma0_events_0"); //打开用户中断
    while (work == 1)
    {
        //sem_wait(&int_sem_rx);//需要在循环外设置第一次中断
        //printf("Start read interrupt !\n");
        read_event(interrupt_fd);                           //获取用户中断
        read_end_addr = (int)read_control(control_base, 0x0008); //read interrupt reg
        write_control(control_base, 0x0000, 0xFFFFFFFF);    //ack interrupt
        sem_post(&int_sem_rx);                              //release signal
        //printf("read_end_addr = %x \n",read_end_addr);
        //lseek(c2h_fd, read_end_addr, SEEK_SET);
        //read(c2h_fd, pData_0, 32*1024);
        //cnt = cnt +1;
        //printf("cnt = %d\n",cnt);
        //printf("Stop read!\n");
    }
    pthread_exit(0);
}

void *rx_process()
{
    struct timeval sstime;
    long time0;
    int cnt;
    while (work == 1)
    {
        sem_wait(&int_sem_rx);
        printf("read_end_addr = %x \n", read_end_addr);
        lseek(c2h_fd, read_end_addr, SEEK_SET);
        read(c2h_fd, pData_0, RX_SIZE);
        cnt = cnt + 1;
        printf("cnt = %d\n", cnt);
        //sem_post(&int_sem_rx);
    }
}

//open such device func:
void dev_open_fun()
{
    control_fd = open_control("/dev/xdma0_bypass");           //打开bypass字符设备
    control_base = mmap_control(control_fd, MAP_BYPASS_SIZE); //获取bypass映射的内存地址

    c2h_fd = open(DEVICE_NAME_C2H_0, O_RDONLY | O_NONBLOCK); //打开pcie c2h设备
    if (c2h_fd == -1)
    {
        printf("PCIe c2h device open failed!\n");
    }
    else
        printf("PCIe c2h device open successful!\n");

    mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, c2h_fd, 0);

    printf("c2h device Memory map successful!\n");

    h2c_fd = open(DEVICE_NAME_H2C_0, O_WRONLY | O_NONBLOCK); //打开pcie  h2c设备
    if (h2c_fd == -1)
    {
        printf("PCIe h2c device open failed!\n");
    }
    else
        printf("PCIe h2c device open successful!\n");

    mmap(0, MAP_SIZE, PROT_WRITE, MAP_SHARED, h2c_fd, 0);

    printf("h2c device Memory map successful!\n");

    work = 1;
}

//culculate the client_num base on channel
int cci(int id){
    return id;
}

//send pthread
void *data_send(int ptd_id)
{
    int id = ptd_id;
    printf("Id为%d的发送线程已创建完毕。\n", id);

    int ret;
    int count = 0;

    while (work == 1)
    {
        sem_wait(&int_sem_rx0);
            ;
        for (int i = 0; i < CLIENT_NUM; i++)
        {
            ret = send(acfd[cci(i)], pack_send[i], RX_SIZE / CLIENT_NUM, 0);
            if (ret < 0)
            {
                printf("第%d次发送 : 对%d客户端 : Send failed!\n", count, i);
                exit(1);
            }
            count++;
        }
        sem_post(&int_sem_rx0);
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

