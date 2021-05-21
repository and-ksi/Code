#include "ana.h"

//需要被初始化以及随环境改变的参数
int client_num = 3;
int channel_num = 8;
int port = 10000;

//功能指示
int work;
static sem_t sem[4];
struct sockaddr_in clientaddr[10] = {0};
FILE *error_fp; //错误报告

//PCIE读取 变量
int c2h_fd[2];
int h2c_fd;
int control_fd;
void *control_base;
int interrupt_fd;
int read_end_addr;
char pData[RX_SIZE / 4];

//socket 变量
int acfd[8];

//统计变量
int socket_count;

/*中断处理进程*/
void *event_process()
{
    interrupt_fd = open_event("/dev/xdma0_events_0"); //打开用户中断
    while (work != -1)
    {
        sem_wait(sem + 0);
        //printf("Start read interrupt !\n");
        read_event(interrupt_fd);                                //获取用户中断
        read_end_addr = (int)read_control(control_base, 0x0008); //read interrupt reg
        write_control(control_base, 0x0000, 0xFFFFFFFF);         //ack interrupt
        sem_post(sem + 1);
    }

}

void *rx_process(int id)
{
    while (work != -1)
    {
        sem_wait(sem + 1);
        //printf("read_end_addr = %x \n", read_end_addr);
        lseek(c2h_fd[id], (read_end_addr - (50 - id * 25) * 1024 * 1024), SEEK_SET);
        read(c2h_fd[id], pData, sizeof(pData)); //10MB
        /* fwrite(pData[0], 4, RX_SIZE / 4, error_fp);
        fclose(error_fp);
        exit(1); */
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

    /*     c2h_fd[1] = open(DEVICE_NAME_C2H_1, O_RDONLY | O_NONBLOCK); //打开pcie c2h_1设备
    if (c2h_fd[1] == -1)
    {
        printf("PCIe c2h_1 device open failed!\n");
    }
    else
        printf("PCIe c2h_1 device open successful!\n");

    mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, c2h_fd[1], 0);

    printf("c2h[1] device Memory map successful!\n"); */

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

//socket和线程创建函数
void socket_create()
{
    int ret;
    int on = 1;
    int i;
    int socket_fd;
    socklen_t len;

    char buf[10] = {'\0'};

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

    for (i = 0; i < client_num; i++)
    {
        printf("等待第%d个客户端连接... %d/%d\n", i + 1, i + 1, client_num);
        *(acfd + i) = accept(socket_fd, (struct sockaddr *)&clientaddr[i], &len);
        if (acfd[i] < 0)
        {
            perror("Accept fail\n");
            exit(1);
        }
        sprintf(buf, "%d", i);
        ret = send(*(acfd + i), buf, sizeof(buf), 0);
        if(ret < 0){
            fprintf(error_fp, "\nSend recv_id failed: buf[%s]\n", buf);
            perror("Send recv_id failed!\n");
            fclose(error_fp);
            exit(1);
        }
        memset(buf, '\0', sizeof(buf));
        recv(*(acfd + i), buf, 10, 0);
        if(strcmp(buf, "ok") != 0){
            printf("Recv OK error!\n");
            fprintf(error_fp, "\nRecv ok failed : buf[%s]\n", buf);
            fclose(error_fp);
            exit(1);
        }
        printf("第%d个客户端已连接!\n", i + 1); //希望能够显示连接的客户端地址
    }
}

void *data_part_send(){
    int ret;
    int location;
    int board_location[1024];
    int board_num;
    int mark_send[2];

    socket_count = 0;

    while(work != -1){
        sem_wait(sem + 2);

        board_location[0] = find_board_head(pData, 0);
        for(board_num = 1; board_num < 1024; board_num++){
            location = find_board_head(pData + board_location[board_num - 1] + 1024, 1);
            if(location == 100){
                board_num--;
                break;
            }
            board_location[board_num] = board_location[board_num - 1] + 1024 + location;
        }
        location = board_num / client_num + 1;

        for(int i = 1; i < client_num; i++){
            ret = send(acfd[i], pData + board_location[i * location],
             4 * (board_location[i * location + 1] - board_location[(i - 1) * location]), 0);
            if(ret < 0){
                printf("Data send failed! socket_count: %d\n", socket_count);
                fprintf(error_fp, "\nData send failed! socket_count: %d\n", socket_count);
                exit(1);
            }
            mark_send[0] = board_num / client_num + 1;
            mark_send[1] = 4 * (board_location[i * location + 1] - board_location[(i - 1) * location]);
            ret = send(acfd[i], mark_send, sizeof(mark_send), 0);
            if(ret < 0){
                printf("Mark_send send failed! : %d, %d\n", mark_send[0], mark_send[1]);
                fprintf(error_fp, "\nMark_send send failed! : %d, %d\n", mark_send[0], mark_send[1]);
                exit(1);
            }
        }
        memset(pData, 0, sizeof(pData));
        sem_post(sem + 0);
        socket_count++;
        memset(mark_send, 0, sizeof(mark_send));
    }
}

int main(int argc, char const *argv[]){
    char sig;
    while(sig != 'y'){
        printf("Port = %d\nClient_num = %d\nChannel_num = %d\nPress'y' to continue...\n", port, client_num, channel_num);
        if(sig != 'y'){
            printf("port, client_num, channel_num\n");
            scanf("%d%d%d", &port, &client_num, &channel_num);
        }
    }
    fprintf(error_fp, "\nPort = %d\nClient_num = %d\nChannel_num = %d\n", port, client_num, channel_num);

    unsigned int pos = 0;
    unsigned int cnontrol_3;

    sem_init(&sem[0], 0, 1);
    sem_init(&sem[1], 0, 0);
    sem_init(&sem[2], 0, 0);

    error_fp = open_error_log();
    work = 1;

    pthread_t rx_thread, event_thread;
    pthread_t part_send_thread;

    dev_open_fun();
    socket_create();

    ptd_create(&rx_thread, -1, rx_process);
    pthread_create(&event_thread, 0, event_process, 0);
    ptd_create(&part_send_thread, 4, data_part_send);

    while (sig != 'o')
    {
        sig = getchar();
        switch (sig)
        {
        case 'w':
            lseek(c2h_fd[0], 0, SEEK_SET);
            read(c2h_fd[0], pData, 4 * 1024);
            printf("Read data successful!\n");

            for (int i = 0; i < 1024; i++)
            {
                printf("The data of address %d is %x \n", i, pData[i]);
            }
            break;

        case 'e':
            socket_count = 0;
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
            fp = fopen("log/a_data.log", "w+");
            if (fp == NULL)
            {
                printf("File open failed!");
                break;
            }
            fwrite(pData, 1, sizeof(pData[1]), fp);
            fclose(fp);
            memset(pData, 0, sizeof(pData[1]));
            break;
        }
    }
    work = -1;

    close(c2h_fd[0]);
    //close(c2h_fd[1]);
    close(h2c_fd);
    close(control_fd);
    close(interrupt_fd);
    for (int i = 0; i < client_num; i++)
    {
        close(acfd[i]);
    }
    return 0;
}