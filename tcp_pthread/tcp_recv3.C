#include "recv_ana2.h"
#include "data_ana.h"

static sem_t int_sem_rxa; //init 1
static sem_t int_sem_rxb; //init 0

char IP[] = "192.168.3.1";
int port = 10000;
int recv_alarm, ana_alarm, global_alarm;
int recv_count;
int socket_fd;
int connect_fd;
int read_length;
int cpy_length;
unsigned int data_pack[2][PACK_SIZE];
int channel[2];

//socket create functon, need global identifier:
//socket_fd IP port connect_fd
void socket_create()
{
    int on = 1;
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
}

//receive pthread
void *pack_recv()
{
    printf("Socket接收线程已创建!\n");

    int ret;
    recv_count = 0;
    socket_create();

    memset(&pack_recved, '0', PACK_SIZE);

    while (global_alarm != -1)
    {
        sem_wait(&int_sem_rxa);
        ret = recv(socket_fd, pack_recved, PACK_SIZE, 0);
        if (ret < 0)
        {
            printf("Recv fail! recv_count: %d \n", recv_count);
            exit(1);
        }
        sem_post(&int_sem_rxb);
        recv_count++;
    }
    return NULL;
}

//要改
void *data_analys()
{
    printf("Ana 线程已创建!\n");

    int length;
    int mark = 0;

    memset(zero_buf, '0', 32);
    channel[0] = channel[1] = -1; //channel不相同，自用，相同，转存另一个

    struct_head_read(pack_recved, 'b');
    length = 32;

    channel[1] = (int)struct_head_read(pack_recved + length, 'c');

    while (global_alarm != -1)
    {
        while (recv_alarm == 0)
            ;
        while (length < PACK_SIZE)
        {
            if (struct_head_read(pack_recved + length, 'l') == 0) //when adc length=0
            {
                break;
            }
            //ana operate
            if (ana_alarm == 0)
            {
                struct_head_read(pack_recved + length, 'f');
            }

            length += (long)struct_head_read(pack_recved + length, 'l');
            mark = 0;
        }
        length = 0;
        memset(pack_recved, '0', PACK_SIZE);
        recv_alarm = 0;
    }
    return NULL;
}

int main()
{
    char sig;

    recv_alarm = 0;
    ana_alarm = 0;
    global_alarm = 0;

    pthread_t recv_ptd, ana_ptd;
    ptd_create(&recv_ptd, 0, (void *(*))pack_recv);
    ptd_create(&ana_ptd, 0, (void *(*))data_analys);

    while (sig != '0')
    {
        sig = getchar();
        switch (sig)
        {
        case '1': //关闭方式不同,1时,等待ana完成再关闭程序
            while (recv_alarm == 1)
                ;
            sig = '0';
            break;

        case '2': //Turn off frame info display
            ana_alarm = 1;
            break;

        default:
            break;
        }
    }
    global_alarm = -1;
    close(connect_fd);
    close(socket_fd);
    return 0;
}
