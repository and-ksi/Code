#include "recv_ana.h"
#include "data_ana.h"

static sem_t sem[2];

//about socket
char IP[] = "192.168.3.4";
int port = 10000;
int socket_fd;
int connect_fd;
int recv_id;
int recv_num = 1;

int work;

//数据读取计数
int recv_count;
int read_length;
int cpy_count;
int time_count;

FILE *error_log, *save;

unsigned int pack_rec[2][PACK_SIZE];
long long timestamp[PACK_SIZE / 40];
int channel;

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

void get_recv_id(){
    char _buf[50] = {'\0'};
    recv(socket_fd, _buf, 50, 0);
    recv_id = atoi(_buf);
    printf("本机id为: %d\n", recv_id);
}

//receive pthread
void *pack_recv()
{
    printf("Socket接收线程已创建!\n");

    int ret;
    recv_count = 0;

    for(int i = 0; i < CHANNEL_NUM; i++){
        ret = i % CLIENT_NUM;
        if(ret == recv_id){
            printf("接收%dCHANNEL_DATA!\n", i);
            recv_num++;
        }
    }

    memset(pack_rec, 0, sizeof(pack_rec));

    while (work != -1)
    {
        sem_wait(sem);
        for(int i = 0; i < recv_num; i++){
            ret = recv(socket_fd, pack_rec[i], sizeof(pack_rec[i]), 0);
            if (ret < 0)
            {
                printf("Recv fail! recv_count: %d \n", recv_count);
                exit(1);
            }
        }
        ret = recv(socket_fd, timestamp, sizeof(timestamp), 0);
        if (ret < 0)
        {
            printf("Recv fail! recv_count: %d \n", recv_count);
            exit(1);
        }
        sem_post(sem + 1);
        recv_count++;
    }
    return NULL;
}

//get mini difference
int difference_func(long long t_stamp){
    int diffrence, diffrence1, m;
    int min = 0;
    diffrence = t_stamp - timestamp[time_count];
    for (m = 1; m < 5; m++)
    {
        diffrence1 = t_stamp - timestamp[time_count + m];
        if (abs(diffrence1) < abs(diffrence))
        {
            diffrence = diffrence1;
        }
    }
    if(m == 4){
        time_count += 4;
        difference_func(t_stamp);
    }else if(diffrence <= DELAY_MAX){
        time_count += m - 1;
        return 1;//指其为有效数据
    }else{
        return 0;//为需要舍弃的数据
    }
}

//临时数据处理
void *data_analys_ls(){
    printf("Ana 线程已创建!\n");

    int mark = 0;
    int ret_len;
    int _length;

    save = fopen("data_save.log", "w+");
    if(save == NULL){
        printf("File data_save.log open failed!\n");
        exit(1);
    }

    cpy_count = 0;

    while (work != -1)
    {
        sem_wait(sem + 1);
        for(int m = 0; m < recv_num; m++){
            while (cpy_count < PACK_SIZE)
            {
                for (int i = 0; i < 1030; i++)
                {
                    if (pack_rec[m][i + cpy_count] == 0xffffffff)
                    {
                        //printf("Get correct head!   %d\n", i);
                        cpy_count += i;
                        break;
                    }
                    else if (i > 1025)
                    {
                        printf("Read head error!    cpy_count: %d\n", cpy_count);
                        for (int e = -20; e < 50; e++)
                        {
                            fprintf(error_log, "%d:   %x\n", e, pack_rec[m][cpy_count + e]);
                        }
                        fclose(error_log);
                        exit(1);
                    }
                }
                //bit_head_read(pack_rec + cpy_count, 'b');
                cpy_count++;

                while (pack_rec[m][cpy_count] != 0xffffffff && pack_rec[m][cpy_count] != 0)
                {
                    //difference_func返回1,保存数据,反之直接跳过该数据
                    if (difference_func(bit_time_read(pack_rec[m] + cpy_count)))
                    {
                        _length = bit_head_read(pack_rec[m] + cpy_count, 'l');
                        fwrite(pack_rec[m] + cpy_count, 4, _length + 3, save);
                        cpy_count += _length + 3;
                    }
                    else
                    {
                        _length = bit_head_read(pack_rec[m] + cpy_count, 'l');
                        cpy_count += _length + 3;
                    }
                }

                if (pack_rec[m][1 + cpy_count] == 0)
                {
                    cpy_count = PACK_SIZE;
                }

                //bit_head_read(pack_rec + cpy_count, 'b');
                //cpy_count++;
                /* printf("仅显示前4个ADC_head info!\n");
            for(int i = 0; i < 4; i++){
                bit_head_read(pack_rec + cpy_count, 'f');
                _length = bit_head_read(pack_rec + cpy_count, 'l');
                cpy_count += _length;
            } */
                /* while(*(pack_rec + cpy_count) != 0xffffffff){
                _length = bit_head_read(pack_rec + cpy_count, 'l');
                cpy_count += _length;
            } */
            }
        }
        
        memset(pack_rec, 0, sizeof(pack_rec));
        sem_post(sem);
    }
}

int main(int argc, char const *argv[])
{
    if(argc > 1){
        port = atoi(argv[1]);
    }
    char sig;

    work = 0;
    time_count = 0;
    sem_init(sem, 0, 1);
    sem_init(sem + 1, 0, 0);

    error_log = fopen("error_log", "w+");
    if (error_log == NULL)
    {
        printf("Error log file open failed!\n");
        exit(1);
    }

    socket_create();
    get_recv_id();

    pthread_t recv_ptd, ana_ptd;
    ptd_create(&recv_ptd, 4, (void *(*))pack_recv);
    ptd_create(&ana_ptd, 0, (void *(*))data_analys_ls);

    while (sig != 'o')
    {
        sig = getchar();
        switch (sig)
        {
        case '1':
            work = 1;
            break;

        default:
            break;
        }
    }

    work = -1;
    fclose(save);
    fclose(error_log);
    close(connect_fd);
    close(socket_fd);
    return 0;
}
