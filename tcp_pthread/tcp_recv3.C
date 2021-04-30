#include "recv_ana.h"
#include "data_ana.h"

static sem_t int_sem_rxa; //init 1
static sem_t int_sem_rxb; //init 0

//about socket
char IP[] = "192.168.3.1";
int port = 10000;
int socket_fd;
int connect_fd;

int global_alarm;

int recv_count;
int read_length;
int cpy_length;
unsigned int data_pack[2][PACK_SIZE];
unsigned int pack_rec[PACK_SIZE];
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

    memset(pack_rec, 0, sizeof(pack_rec));

    while (global_alarm != -1)
    {
        sem_wait(&int_sem_rxa);
        ret = recv(socket_fd, pack_rec, sizeof(pack_rec), 0);
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

    int cpy_length;
    int mark = 0;
    int ret_len;

    channel[0] = channel[1] = -1; //channel不相同，自用，相同，转存另一个

    bit_head_read(pack_rec, 'b');
    cpy_length = 1;

    channel[1] = (int)bit_head_read(pack_rec + cpy_length, 'c');

    while (global_alarm != -1)
    {
        sem_wait(&int_sem_rxb);
        while (cpy_length < PACK_SIZE)
        {
            //when adc length=0
            if ((ret_len = bit_head_read(pack_rec + cpy_length, 'l')) == 0) 
            {
                break;
            }
            //show adc frame head info
            if (global_alarm == 1)
            {
                bit_head_read(pack_rec + cpy_length, 'f');
            }
            if(channel[0] = bit_head_read(pack_rec + cpy_length, 'c') == channel[1]){
                mark = 1;
            }
            //ana operation

            cpy_length += bit_head_read(pack_rec + cpy_length, 'l');
            mark = 0;
        }
        memset(pack_rec, '0', PACK_SIZE);
        sem_post(&int_sem_rxa);
        cpy_length = 0;
    }
    return NULL;
}

int main()
{
    char sig;

    global_alarm = 0;
    sem_init(&int_sem_rxa, 0, 0);
    sem_init(&int_sem_rxb, 0, 0);

    pthread_t recv_ptd, ana_ptd;
    ptd_create(&recv_ptd, 0, (void *(*))pack_recv);
    ptd_create(&ana_ptd, 0, (void *(*))data_analys);

    while (sig != '0')
    {
        sig = getchar();
        switch (sig)
        {
        case '1':
            global_alarm = 1;
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
