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
        
        sem_wait(&sem[1]);
        
        //printf("Start read interrupt !\n");
        read_event(interrupt_fd);             //获取用户中断

        read_end_addr = (int)read_control(control_base, 0x0008); //read interrupt reg
        
        write_control(control_base, 0x0000, 0xFFFFFFFF);         //ack interrupt
        
        sem_post(&sem[0]); //release signal
        
    }
    pthread_exit(0);
}

void *rx_process()
{
    while (work == 1)
    {

        sem_wait(&sem[0]);

        sem_wait(&sem[2]);
        
        //printf("read_end_addr = %x \n", read_end_addr);
        lseek(c2h_fd[0], (read_end_addr - RX_SIZE), SEEK_SET);
        
        read(c2h_fd[0], pData[0], sizeof(pData[0])); //10MB
        lseek(c2h_fd[1], (read_end_addr - RX_SIZE / 2), SEEK_SET);
        read(c2h_fd[1], pData[1], sizeof(pData[1])); //10MB
        sem_post(&sem[1]);
        sem_post(&sem[3]);
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

    int test = 0;

    while (work == 1)
    {
        sem_wait(sem + 3);
        for(int k = 0; k < 1; k++){
            while (cpy_count < RX_SIZE / 8)
            {
                for (int i = 0; i < 2000; i++) //检测board_head位置
                {
                    if (*(pData[k] + cpy_count + i) == 0x0000f00f)
                    {
                        printf("Get correct head!   %d\n", i);
                        cpy_count += i; //conduct 0x040000ff and board_head
                        break;
                    }
                    else if (i > 1998)
                    {
                        printf("Read head error!\n");
                        exit(1);
                    }
                }
               /*  for(int i = -20; i < 100; i++){
                    printf("%x\n", *(pData[0] + i));
                } */
                /* printf("%x\n", *(pData[k] + cpy_count));
                printf("%x\n", *(pData[k] + cpy_count + 1));
                printf("%x\n", *(pData[k] + cpy_count + 2)); */
                //bit_head_read(pData[k] + cpy_count, 'b');
                cpy_count += 2;
                /* for(int i = 0; i < 50; i++){
                    if(bit_head_read(pData[k] + cpy_count + i, 'e') == 0x3f &&
                     bit_head_read(pData[k] + cpy_count + i, 'c') == 0x01){
                        cpy_count += i;
                        break;
                    }
                    if(i == 49){
                        printf("Can not find adc head!\n");
                        exit(1);
                    }
                } */
                /* cpy_count1 = cpy_count;
                printf("\n仅显示前20个ADC_head info!\n");
                for (int i = 0; i < 4; i++)
                {
                    printf("%x\n", *(pData[k] + cpy_count));
                    printf("%x\n", *(pData[k] + cpy_count + 1));
                    printf("%x\n", *(pData[k] + cpy_count + 2));
                    bit_head_read(pData[k] + cpy_count, 'f');
                    _length = bit_head_read(pData[k] + cpy_count, 'l');
                    cpy_count += _length + 1;
                }
                // fwrite(pData[0], 1, RX_SIZE, fp[1]);
                // fclose(fp[1]);
                // exit(1);
                cpy_count = cpy_count1; */
                cpy_count1 = 0;
                while (*(pData[k] + cpy_count) != 0x66666666 &&
                 *(pData[k] + cpy_count) != 0x7000f000)
                {
                    
                    _length = bit_head_read(pData[k] + cpy_count, 'l');
                    _channle = bit_head_read(pData[k] + cpy_count, 'c');
                    //printf("%d\n", _channle);
                    _timestamp = bit_time_read(pData[k] + cpy_count);
                    cpy_count += 3;
                    
                    for (int i = 0; i < _length - 2; i++)
                    {

                        if(bit_data_read(pData[k] + cpy_count + i, 'd', 1) == 0){

                            if(bit_data_read(pData[k] + cpy_count + i, 'd', 0) == 0){

                                break;
                            }else{
                                
                                fprintf(fp[_channle], "%08d    %09.8f\n", 2 * i * 8,
                                 bit_float_read(pData[k] + cpy_count + i, 0));

                                 break;
                            }
                        }
                        // printf("%x\n", *(pData[k] + cpy_count + i));
                        // printf("%8d\n", bit_data_read(pData[k] + cpy_count + i, 'd', 0));
                        // printf("%8d\n\n", bit_data_read(pData[k] + cpy_count + i, 'd', 1));

                        fprintf(fp[_channle], "%08d    %09.8f\n", 2 * i * 8,
                         bit_float_read(pData[k] + cpy_count + i, 0));
                        //printf("sllslsl\n");

                        fprintf(fp[_channle], "%08d    %09.8f\n", 2 * i * 8 + 8,

                         bit_float_read(pData[k] + cpy_count + i, 1));

                    }
                    // fclose(fp[0]);
                    // exit(1);
                    cpy_count += _length - 2;
                    
                }

                for(int i = -20; i < 100; i++){
                    printf("%x\n", *(pData[0] + cpy_count + i));
                }

                while (*(pData[k] + cpy_count) == 0x66666666
                 || *(pData[k] + cpy_count) == 0x7000f000) //确定是否读到board最后
                {
                    cpy_count++;
                }
                for(int i = -10; i < 50; i++){
                    printf("%x\n", *(pData[k] + cpy_count + i));
                }
                printf("stop\n");
                break;
            }
        }
        test++;
        if(test > 4){
            printf("test: %d\n", test);
            for(int i = 0; i < CHANNEL_NUM; i++)
        {
            fclose(fp[i]);
        }
        exit(1);
        }
        
        sem_post(sem + 2);
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
    sem_init(sem, 0, 0);
    sem_init(sem + 1, 0, 1);
    sem_init(sem + 2, 0, 1);
    sem_init(sem + 3, 0, 0);
    char sig;

    //debug参数?
    unsigned int pos = 0;
    unsigned int cnontrol_3;

    pthread_t event_thread, rx_thread[2];
    pthread_t part_ptd, send_ptd;

    dev_open_fun();

    work = 1;

    pthread_create(&event_thread, NULL, (void *(*)(void *))event_process, NULL);
    
    pthread_create(rx_thread, 0, (void *(*)(void *))rx_process, 0);
    pthread_create(&part_ptd, 0, (void *(*)(void *))show_func, 0);
    


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