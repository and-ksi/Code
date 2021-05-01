#include "recv_ana.h"

int work;

int c2h_fd;
int h2c_fd;
int control_fd;
void *control_base;
int interrupt_fd;
int read_end_addr;

unsigned int pData[2][RX_SIZE / 4]; //10M / 4 * 32 bit     count
                                    //10M / 4 * 4 MB       size

static sem_t int_sem_rxa;//init 1
static sem_t int_sem_rxb;//init 0
static sem_t int_sem_rxc;//init 1
static sem_t int_sem_rxd;//init 0

//准备发往各个客户端的二维数组
unsigned int pack_send[CHANNEL_NUM][PACK_SIZE];

//socket所需变量
int acfd[CLIENT_NUM];
int port = 10000;
int socket_fd;
struct sockaddr_in clientaddr[CLIENT_NUM];

int total_count;
int past_count[CLIENT_NUM] = {0};

/*中断处理进程*/
void *event_process()
{
    interrupt_fd = open_event("/dev/xdma0_events_0"); //打开用户中断
    while (work == 1)
    {
        sem_wait(&int_sem_rxa);
        //printf("Start read interrupt !\n");
        read_event(interrupt_fd);                           //获取用户中断
        read_end_addr = (int)read_control(control_base, 0x0008); //read interrupt reg
        write_control(control_base, 0x0000, 0xFFFFFFFF);    //ack interrupt
        //printf("read_end_addr = %x \n",read_end_addr);
        //lseek(c2h_fd, read_end_addr, SEEK_SET);
        //read(c2h_fd, pData[0], 32*1024);
        //cnt = cnt +1;
        //printf("cnt = %d\n",cnt);
        //printf("Stop read!\n");
        sem_post(&int_sem_rxb); //release signal
    }
    pthread_exit(0);
}

void *rx_process()
{
    struct timeval sstime;
    long time0;

    while (work == 1)
    {
        sem_wait(&int_sem_rxb);
        //while(read all of the buf :50MB)
        printf("read_end_addr = %x \n", read_end_addr);
        lseek(c2h_fd, read_end_addr, SEEK_SET);
        read(c2h_fd, pData[0], RX_SIZE);
        printf("cnt: %d\n", total_count);
        total_count++;
        sem_post(&int_sem_rxc);
        sem_post(&int_sem_rxa); //当有两个数组交替读取时,条件执行post:a
                                //因为暂时不需要交替,无条件执行post:a
        //while break;
        //sem_post(&int_sem_rxa);
    }
    return NULL;
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
}

//culculate the client_num base on channel
int cci(int id){
    return id;
}

//data send function; how to switch client and pack_send?
void *data_send_func(int *count__){
    int ret;
    while(work == 1){
        sem_wait(&int_sem_rxd);
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
        sem_post(&int_sem_rxc);
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
void part_operation(unsigned int *ddata, int si, int *_cpy_cnt, int *p_cnt){
    int _channel_, _length;
    while (*_cpy_cnt < RX_SIZE / 4){
        if ((_length = bit_head_read(ddata + *_cpy_cnt, 'l')) == 0)
        {
            break;
        }
        _channel_ = bit_head_read(ddata + *_cpy_cnt, 'c');
        memcpy(pack_send[cci(_channel_)] + *p_cnt, ddata + *_cpy_cnt, 4 * _length);
        *(p_cnt + cci(_channel_)) += _length + 1;
        *_cpy_cnt += _length + 1;
    }
}

//part data to each client
//wait33 --> post1;post2
void *data_part(){
    int cpy_count;
    int pid;

    memset(&pack_send, 0, sizeof(pack_send));
    sem_wait(&int_sem_rxc);//only for add board info to pack_send
    for(int i = 0; i < CLIENT_NUM; i++){
        pack_send[i][0] = pData[0][0];
        past_count[i]++;
    }
    cpy_count++;
    sem_post(&int_sem_rxc);
    sem_post(&int_sem_rxc);
    while(work == 1){
        pid = pid & 1;
        sem_wait(&int_sem_rxc);//等待添加pData[1]...的处理
        sem_wait(&int_sem_rxc);
        part_operation(pData[0], RX_SIZE / 4, &cpy_count, past_count);
        sem_post(&int_sem_rxb);
        sem_post(&int_sem_rxd);
        cpy_count = 0;
        pid++;
    }
    return NULL;
}

int main(){
    sem_init(&int_sem_rxa, 0, 1);
    sem_init(&int_sem_rxb, 0, 0);
    sem_init(&int_sem_rxc, 0, 1);
    sem_init(&int_sem_rxd, 0, 0);

    char sig;
    unsigned int pos = 0;
    unsigned int cnontrol_3;

    pthread_t event_thread, rx_thread;
    pthread_t part_ptd, send_ptd;

    dev_open_fun();
    socket_create();
    work = 1;

    pthread_create(&event_thread, NULL, (void *(*)(void *))event_process, NULL);
    pthread_create(&rx_thread, NULL, (void *(*)(void *))rx_process, NULL);
    ptd_create(&part_ptd, CPU_CORE - 2, (void *)data_part);
    ptd_create(&send_ptd, CPU_CORE - 1, (void *)data_send_func);

    while(sig != 'o'){
        sig = getchar();
        switch (sig)
        {
        case 'w':
            lseek(c2h_fd, pos, SEEK_SET);
            read(c2h_fd, pData[0], 4 * 1024);
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

    close(c2h_fd);
    close(h2c_fd);
    close(control_fd);
    close(interrupt_fd);
    return 0;
}