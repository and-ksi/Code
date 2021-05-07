#include "recv_ana.h"

int work;

int c2h_fd[2];
int h2c_fd;
int control_fd;
void *control_base;
int interrupt_fd;
int read_end_addr;

unsigned int pData[2][RX_SIZE / 8]; //25M / 4 * 32 bit     count
                                    //10M / 4 * 4 MB       size

static sem_t sem[4];

//准备发往各个客户端的二维数组
unsigned int pack_send[CLIENT_NUM][PACK_SIZE];

//socket所需变量
int acfd[CLIENT_NUM];
int port = 10000;
int socket_fd;
struct sockaddr_in clientaddr[CLIENT_NUM];

//数据分发计数
int total_count;
int past_count[CLIENT_NUM] = {0};//pasted_count
int cpy_count;

/*中断处理进程*/
void *event_process()
{
    interrupt_fd = open_event("/dev/xdma0_events_0"); //打开用户中断
    while (work == 1)
    {
        sem_wait(&sem[0]);
        sem_wait(&sem[0]);
        //printf("Start read interrupt !\n");
        read_event(interrupt_fd);                           //获取用户中断
        read_end_addr = (int)read_control(control_base, 0x0008); //read interrupt reg
        write_control(control_base, 0x0000, 0xFFFFFFFF);    //ack interrupt
        

        sem_post(&sem[1]); //release signal
        sem_post(&sem[1]);
    }
    pthread_exit(0);
}

void *rx_process(int id)
{
    while (work == 1)
    {
        sem_wait(&sem[4]);
        sem_wait(&sem[1]);
        //printf("read_end_addr = %x \n", read_end_addr);
        lseek(c2h_fd[id], (read_end_addr - (50 - id * 25) * 1024 * 1024), SEEK_SET);
        read(c2h_fd[id], pData[id], sizeof(pData[id])); //10MB
        sem_post(&sem[0]);
        sem_post(&sem[2]);
    }
    pthread_exit(0);
}

//open such device func:
void dev_open_fun()
{
    control_fd = open_control("/dev/xdma0_bypass");           //打开bypass字符设备
    control_base = mmap_control(control_fd, MAP_BYPASS_SIZE); //获取bypass映射的内存地址

    c2h_fd[0] = open(DEVICE_NAME_C2H_0, O_RDONLY | O_NONBLOCK); //打开pcie c2h设备
    if (c2h_fd[0] == -1)
    {
        printf("PCIe c2h device open failed!\n");
    }
    else
        printf("PCIe c2h device open successful!\n");

    mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, c2h_fd[0], 0);

    printf("c2h0 device Memory map successful!\n");

    c2h_fd[1] = open(DEVICE_NAME_C2H_1, O_RDONLY | O_NONBLOCK); //打开pcie c2h_1设备
    if (c2h_fd[1] == -1)
    {
        printf("PCIe c2h_1 device open failed!\n");
    }
    else
        printf("PCIe c2h_1 device open successful!\n");

    mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, c2h_fd[1], 0);

    printf("c2h[1] device Memory map successful!\n");

    h2c_fd = open(DEVICE_NAME_H2C_0, O_WRONLY | O_NONBLOCK); //打开pcie  h2c设备
    if (h2c_fd == -1)
    {
        printf("PCIe h2c device open failed!\n");
    }
    else
        printf("PCIe h2c device open successful!\n");

    mmap(0, MAP_SIZE, PROT_WRITE, MAP_SHARED, h2c_fd, 0);

    printf("h2c device Memory map successful!\n");
}

//culculate the client_num base on channel
int cci(int id){
    return id;
}

//data send function; how to switch client and pack_send?
void *data_send_func(int *count__){
    int ret;
    while(work == 1){
        sem_wait(&sem[3]);
        for (int i = 0; i < CLIENT_NUM; i++)
        {
            ret = send(acfd[i], pack_send[i], past_count[i], 0);
            if (ret < 0)
            {
                printf("第%d次发送 : 对%d客户端 : Send failed!\n", *count__, i);
            }
        }
        *count__++;
        memset(past_count, 0, sizeof(past_count));
        sem_post(&sem[2]);
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

//part operation function
void part_operation(unsigned int *ddata, int si){
    int _channel, _length;
    while (cpy_count < si){
        for (int i = 0; i < 1030; i++)//检测board_head位置
        {
            if (*(ddata + cpy_count + i) == 0x00000000)
            {
                printf("Get correct head!   %d\n", i);
                cpy_count += i;//conduct 0x040000ff and board_head
                break;
            }else if (i > 1025)
            {
                printf("Read head error!\n");
                exit(1);
            }
        }
        for(int i = 0; i < CLIENT_NUM; i++)//将board_head写入待发送数组
        {
            pack_send[i][past_count[i]] = *(ddata + cpy_count);
            past_count[i]++;
        }
        cpy_count++;//指向board_head下第一个adc_head第一行
        for(int i = cpy_count; cpy_count - i < 1023;)//将board下所有adc_data存入对应内存
        {
            _length = bit_head_read(ddata + cpy_count, 'l');
            if (_length == 0 || _length < 3)
            {
                printf("Data read error!\n");
                break;
            }
            _channel = bit_head_read(ddata + cpy_count, 'c');
            memcpy(pack_send[cci(_channel)] + past_count[cci(_channel)],
             ddata + cpy_count, _length + 3);
            past_count[cci(_channel)] += _length + 3;//length 包括数据数量以及两个时间戳
            cpy_count += _length + 3;
            while(*(ddata + cpy_count) == 0)//确定是否读到board最后
            {
                cpy_count++;
            }
        }
        for(int i = 0; i < CLIENT_NUM; i++)//接收端分析数据时读到8f结束当前board
        {
            *(pack_send[i] + past_count[i]) = 0xffffffff;
            past_count[i]++;
        }
    }
    cpy_count = 0;
}

//part data to each client
void *data_part(){
    cpy_count = 0;

    memset(&pack_send, 0, sizeof(pack_send));
    while(work == 1){
        sem_wait(&sem[2]);
        sem_wait(&sem[2]);
        sem_wait(&sem[2]);
        part_operation(pData[0], sizeof(pData[0]));
        part_operation(pData[1], sizeof(pData[1]));
        sem_post(&sem[3]);
        sem_post(&sem[3]);
        sem_post(&sem[3]);
    }
    return NULL;
}

int main(){
    sem_init(sem , 0, 2);
    sem_init(sem + 1, 0, 0);
    sem_init(sem + 2, 0, 1);
    sem_init(sem + 3, 0, 2);//注意让两个read先使用这个信号,否则出bug

    char sig;

    //debug参数?
    unsigned int pos = 0;
    unsigned int cnontrol_3;

    pthread_t event_thread, rx_thread[2];
    pthread_t part_ptd, send_ptd;

    dev_open_fun();
    socket_create();
    work = 1;

    for(int i = 0; i < 2; i++){
        pthread_create(rx_thread + i, 0, (void *(*)(void *))rx_thread, &i);
    }
    pthread_create(&event_thread, NULL, (void *(*)(void *))event_process, NULL);
    ptd_create(&part_ptd, CPU_CORE - 2, (void *)data_part);
    ptd_create(&send_ptd, CPU_CORE - 1, (void *)data_send_func);

    while(sig != 'o'){
        sig = getchar();
        switch (sig)
        {
        case 'w':
            lseek(c2h_fd[0], 0, SEEK_SET);
            read(c2h_fd[0], pData[0], 4 * 1024);
            printf("Read data successful!\n");

            for (int i = 0; i < 1024; i++)
            {
                printf("The data of address %d is %x \n", i, pData[0][i]);
            }
            break;

        case 'e':
            total_count = 0;
            break;

        case 'r':
            write_control(control_base, 0x0000, 0xFFFFFFFF);
            break;

        case 't':
            write_control(control_base, 0x0004, 0xFFFFFFFF);
            break;

        case 'y':
            cnontrol_3 = read_control(control_base, 0x0008);
            printf("read_end_addr = %x \n", cnontrol_3);
            break;

        case 'u':
            pos = pos + 1024;
            printf("test_read_end_addr = %x \n", pos);
            break;

        case 'i':
            pos = 0;
            printf("test_read_end_addr = %x \n", pos);
            break;

        case 'g':
            memcpy(pData[1], pData[0], sizeof(pData[1]));
            FILE *fp;
            fp = fopen("data.log", "w+");
            if (fp == NULL)
            {
                printf("File open failed!");
                break;
            }
            fwrite(pData[1], 1, sizeof(pData[1]), fp);
            fclose(fp);
            memset(pData[1], 0, sizeof(pData[1]));
            break;
        }
    }
    work = 0;

    close(c2h_fd[0]);
    close(c2h_fd[1]);
    close(h2c_fd);
    close(control_fd);
    close(interrupt_fd);
    for(int i = 0; i < CLIENT_NUM; i++){
        close(acfd[i]);
    }
    close(socket_fd);
    return 0;
}