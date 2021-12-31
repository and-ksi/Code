#include "ana.h"

//从socket接收并进行改变
int channel_num = 8;
int min_channel = 0;
int end_location;
int board_num = 1024;
int client_num = 1;

//功能控制
int work;
FILE *error_fp; //错误报告
FILE *log_save;
int file_count = 0;

//socket need
char ip[32] = "192.168.3.4";
int socket_fd, connect_fd;
int port = 10000;
int recv_id;
unsigned int recved_pack[PACK_SIZE];

//计数
int recv_count;
int valid_example_count = 0;

static sem_t sem[5];
pthread_mutex_t mute;

typedef struct example_before
{
    int m_board_num;
    int start_address;
    int m_mark;
} EXAMPLE_B;

//socket create functon, need global identifier:
//socket_fd IP port connect_fd
void socket_create()
{
    int on = 1;
    int ret;
    char buf[32] = {0};
    int recv_info[4];
    socklen_t sendbuflen = 0;
    socklen_t len = sizeof(sendbuflen);

    // debug
    // sendbuflen = RX_SIZE;
    // setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (void *)&sendbuflen, len);
    // getsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (void *)&sendbuflen, &len);
    // printf("debug: now,sendbuf:%d\n", sendbuflen);

    struct sockaddr_in serveraddr = {0};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serveraddr.sin_addr.s_addr);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        printf("Socket fail!\n");
        exit(1);
    }

    // debug
    // getsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF, (void *)&sendbuflen, &len);
    // printf("default,sendbuf:%d\n", sendbuflen);

    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    connect_fd = connect(socket_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (connect_fd < 0)
    {
        printf("Connect fail!\n");
        close(socket_fd);
        exit(1);
    }
    for (on = 0; on < sizeof(recv_info);)
    {
        ret = recv(socket_fd, recv_info + on, sizeof(recv_info), 0);
        if (ret != sizeof(recv_info))
        {
            fprintf(error_fp, "\nRecv recv_info failed: [0]%d [1]%d\n", recv_info[0], recv_info[1]);
            printf("Recv recv_info failed!\n");
            fclose(error_fp);
            exit(1);
        }
        on += ret;
    }

    recv_id = recv_info[0];
    channel_num = recv_info[1];
    min_channel = recv_info[2];
    client_num = recv_info[3];
    fprintf(error_fp, "\nrecv_id: %d    channel_num: %d\nmin_channel: %d    client_num: %d\n\n", recv_id, channel_num, min_channel, client_num);
    memset(buf, 0, 32);
    sprintf(buf, "ok");
    ret = send(socket_fd, buf, 2, 0);
    if (ret != 2)
    {
        fprintf(error_fp, "\nSend ok failed: buf[%s]\n", buf);
        printf("Send ok failed!\n");
        fclose(error_fp);
        exit(1);
    }
    fflush(error_fp);
}

void *recv_func()
{
    int ret;
    int mark_recv[2];
    int recved_ll;
    recv_count = 0;

    int recv_num;
    ret = BOARD_NUM / client_num + 1;
    if (recv_id == client_num - 1)
    {
        recv_num = (BOARD_NUM - (client_num - 1) * ret) * 1024;
    }
    else
    {
        recv_num = ret * 1024;
    }

    while (work != -1)
    {
        sem_wait(sem + 0);
        printf("debug: start wait for recving data\n");
        for (recved_ll = 0; recved_ll < recv_num * 4;)
        {
            ret = recv(socket_fd, (char *)recved_pack + recved_ll, recv_num * 4 - recved_ll, 0);
            if (ret < 0)
            {
                printf("Recv failed! recv_count: %d, recved_ll: %d\n", recv_count, recved_ll);
                fprintf(error_fp, "\nRecv failed! recv_count: %d, recved_ll: %d\n", recv_count, recved_ll);
                write_error_log(&error_fp, recved_pack, 1);
                fclose(error_fp);
                exit(1);
            }

            recved_ll += ret;
        }
        printf("debug: recv_ll = %d\n", recved_ll);

        while (work == 1) ;
        recv_count++;
        sem_post(sem + 1);
        printf("debug: recv data success: %d\n", recv_count);
    }
}

void *data_analys(id)
{
    int ret;
    int loca;
    int board_location[200] = {0};
    int recv_num;
    int start_num, end_num;
    int start_address, end_address;

    pthread_attr_t attr;
    pthread_t ana_ptd[3];
    EXAMPLE_B _me[3];

    //debug
    for (int i = 0; i < 1; i++)
    {
        //ptd_create(ana_ptd + i, i + 1, example_analys, _me + i, 0);
    }

    while (work != -1)
    {
        board_num = BOARD_NUM;
        sem_wait(sem + 1);
        //printf("debug: data_analys start\n");

        ret = find_board_head(recved_pack + PACK_SIZE - 1024, 0, 0);
        if (ret == 100)
        {
            fprintf(error_fp, "cant find the last board! recv_count:%d\n", recv_count);
            write_data_error_log(&error_fp, recved_pack, PACK_SIZE, 0);
            exit(1);
        }
        // debug
        //recv_num = board_num / 3 + 1;
        recv_num = board_num;

        //debug
        for (int i = 0; i < 1; i++)
        {

            start_num = i * recv_num;

            start_address = start_num * 1024 + find_board_head(recved_pack + start_num * 1024, 0, 0);
            //printf("debug: start_address: %d\n", start_address);
            (_me + i)->start_address = start_address;
            if (i == 2)
            {
                (_me + i)->m_board_num = board_num - 2 * recv_num;
            }
            else
            {
                (_me + i)->m_board_num = recv_num;
            }
            (_me + i)->m_mark = i;
        }
        //printf("debug: analys start\n");
        sem_post(sem + 2);
        sem_post(sem + 3);
        sem_post(sem + 4);

        sem_wait(sem + 1);
        //sem_wait(sem + 1);
        //sem_wait(sem + 1);

        //printf("debug: analys end\n");
        //debug
        //exit(1);

        memset(recved_pack, 0, sizeof(recved_pack));
        sem_post(sem + 0);
    }
}

int main()
{
    error_fp = open_error_log();
    fprintf(error_fp, "This is recv.c error.log!\n");
    char sig;
    while (sig != 'y')
    {
        printf("Port = %d\nIP = %s\nPress'y' to continue...\n", port, ip);
        scanf("%s", &sig);
        if (sig != 'y')
        {
            memset(ip, '\0', sizeof(ip));
            printf("port:\n");
            scanf("%d", &port);
            printf("ip(no space):\n");
            scanf("%s", ip);
        }
    }
    fprintf(error_fp, "\nPort = %d\nip = %s\n\n", port, ip);
    fflush(error_fp);

    work = 0;
    pthread_t recv_ptd, ana_ptd;
    sem_init(sem + 0, 0, 1);
    sem_init(sem + 1, 0, 0);
    sem_init(sem + 2, 0, 0);
    sem_init(sem + 3, 0, 0);
    sem_init(sem + 4, 0, 0);
    log_save = NULL;

    socket_create();
    printf("This is the %d client!\n", recv_id);
    fprintf(error_fp, "This is the %d client!\n", recv_id);
    ptd_create(&recv_ptd, 0, recv_func, 0, 0);
    ptd_create(&ana_ptd, 0, data_analys, 0, 0);

    while (sig != 'o')
    {
        sig = getchar();
        switch (sig)
        {
        case 'w':
            work = 1;

        default:
            break;
        }
    }

    close(socket_fd);
    close(connect_fd);
    fclose(error_fp);
    fclose(log_save);
    exit(1);
}