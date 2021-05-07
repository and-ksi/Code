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
unsigned int pack_send[CHANNEL_NUM][PACK_SIZE];

//socket所需变量
int acfd[CLIENT_NUM];
int port = 10000;
int socket_fd;
struct sockaddr_in clientaddr[CLIENT_NUM];

//数据分发计数
int total_count;
int past_count[CLIENT_NUM] = {0}; //pasted_count
int cpy_count;

/*中断处理进程*/
void *event_process()
{
    interrupt_fd = open_event("/dev/xdma0_events_0"); //打开用户中断
    while (work == 1)
    {
        sem_wait(&sem[0]);
        //printf("Start read interrupt !\n");
        read_event(interrupt_fd);                                //获取用户中断
        read_end_addr = (int)read_control(control_base, 0x0008); //read interrupt reg
        write_control(control_base, 0x0000, 0xFFFFFFFF);         //ack interrupt

        sem_post(&sem[1]); //release signal
    }
    pthread_exit(0);
}

void *rx_process()
{
    while (work == 1)
    {
        sem_wait(&sem[0]);
        sem_wait(&sem[0]);
        //printf("read_end_addr = %x \n", read_end_addr);
        lseek(c2h_fd[0], (read_end_addr - RX_SIZE), SEEK_SET);
        read(c2h_fd[0], pData[0], sizeof(pData[0])); //10MB
        lseek(c2h_fd[1], (read_end_addr - RX_SIZE / 2), SEEK_SET);
        read(c2h_fd[1], pData[1], sizeof(pData[1])); //10MB
        sem_post(&sem[1]);
        sem_post(&sem[1]);
    }
    pthread_exit(0);
}

//show info
void *show_func(){
    cpy_count = 0;
    int _length;
    int _channle;
    int cpy_count1;
    long long _timestamp;
    char buf[100] = {'\0'};
    FILE *fp[CHANNEL_NUM];
    for(int i = 0; i < CHANNEL_NUM; i++){
        sprintf(buf, "log/c_%d.log", i);
        fp[i] = fopen(buf, "w+");
    }

    while (work == 1)
    {
        sem_wait(sem);
        for(int k = 0; k < 2; k++){
            while (cpy_count < RX_SIZE / 8)
            {
                for (int i = 0; i < 1030; i++) //检测board_head位置
                {
                    if (*(pData[k] + cpy_count + i) == 0x00000000
                     && *(pData[k] + cpy_count + i + 1) != 0)
                    {
                        printf("Get correct head!   %d\n", i);
                        cpy_count += i; //conduct 0x040000ff and board_head
                        break;
                    }
                    else if (i > 1025)
                    {
                        printf("Read head error!\n");
                        exit(1);
                    }
                }
                bit_head_read(pData[k] + cpy_count, 'b');
                cpy_count++;
                cpy_count1 = cpy_count;
                printf("仅显示前4个ADC_head info!\n");
                for (int i = 0; i < 4; i++)
                {
                    bit_head_read(pData[k] + cpy_count, 'f');
                    _length = bit_head_read(pData[k] + cpy_count, 'l');
                    cpy_count += _length + 3;
                }
                cpy_count = cpy_count1;
                while (*(pData[k] + cpy_count) != 0 &&
                 *(pData[k] + cpy_count + 1) != 0)
                {
                    _length = bit_head_read(pData[k] + cpy_count, 'l');
                    _channle = bit_head_read(pData[k] + cpy_count, 'c');
                    _timestamp = bit_head_read(pData[k] + cpy_count, 't');
                    cpy_count += 3;
                    for(int i = 0; i < _length; i++)
                    {
                        fprintf(fp[_channle], "%018d    %09.8f\n",
                         _timestamp + i, bit_float_read(pData[k] + cpy_count + i, 0));
                    }
                    cpy_count += _length;
                }
                while (*(pData[k] + cpy_count) == 0
                 && *(pData[k] + cpy_count + 1) != 0) //确定是否读到board最后
                {
                    cpy_count++;
                }
            }
        }
        exit(1);
        sem_post(sem + 1);
    }
    
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

int main()
{
    sem_init(sem, 0, 2);
    sem_init(sem + 1, 0, 0);
    sem_init(sem + 2, 0, 1);
    sem_init(sem + 3, 0, 2); //注意让两个read先使用这个信号,否则出bug

    char sig;

    //debug参数?
    unsigned int pos = 0;
    unsigned int cnontrol_3;

    pthread_t event_thread, rx_thread[2];
    pthread_t part_ptd, send_ptd;

    dev_open_fun();

    work = 1;

    pthread_create(rx_thread, 0, (void *(*)(void *))rx_thread, 0);
    pthread_create(&event_thread, NULL, (void *(*)(void *))event_process, NULL);

    while (sig != 'o')
    {
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
    for (int i = 0; i < CLIENT_NUM; i++)
    {
        close(acfd[i]);
    }
    close(socket_fd);
    return 0;
}