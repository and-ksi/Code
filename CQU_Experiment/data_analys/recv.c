#include "ana.h"

//从socket接收并进行改变
int channel_num;
int min_channel;
int end_location;
int board_num = 1024;
int client_num;

//功能控制
int work;
FILE *error_fp; //错误报告
FILE *log_save;
int file_count;

//socket need
char ip[32] = "192.168.3.4";
int socket_fd, connect_fd;
int port = 10000;
int recv_id;
unsigned int recved_pack[PACK_SIZE];

//计数
int recv_count;
int valid_example_count = 0;

static sem_t sem[2];

typedef struct example_before{
    int m_board_num;
    int start_address;
    int m_mark;
}EXAMPLE_B;

//socket create functon, need global identifier:
//socket_fd IP port connect_fd
void socket_create()
{
    int on = 1;
    int ret;
    char buf[32] = {0};
    int recv_info[4];

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
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    connect_fd = connect(socket_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (connect_fd < 0)
    {
        printf("Connect fail!\n");
        close(socket_fd);
        exit(1);
    }
    ret = recv(socket_fd, recv_info, sizeof(recv_info), 0);
    if(ret < 0){
        fprintf(error_fp, "\nRecv recv_info failed: [0]%d [1]%d\n", recv_info[0], recv_info[1]);
        printf("Recv recv_info failed!\n");
        fclose(error_fp);
        exit(1);
    }
    recv_id = recv_info[0];
    channel_num = recv_info[1];
    min_channel = recv_info[2];
    client_num = recv_info[3];
    memset(buf, 0, 32);
    sprintf(buf, "ok");
    ret = send(socket_fd, buf, 2, 0);
    if(ret < 0){
        fprintf(error_fp, "\nSend ok failed: buf[%s]\n", buf);
        printf("Send ok failed!\n");
        fclose(error_fp);
        exit(1);
    }
}

void *recv_func(){
    int ret;
    int mark_recv[2];
    while(work != -1){
        sem_wait(sem + 0);
        printf("debug: start wait for recving data\n");
        ret = recv(socket_fd, recved_pack, sizeof(recved_pack), 0);
        if(ret < 0){
            printf("Recv failed! recv_count: %d\n", recv_count);
            fprintf(error_fp, "\nRecv failed! recv_count: %d\n", recv_count);
            write_error_log(error_fp, recved_pack, 1);
            fclose(error_fp);
            exit(1);
        }

        //debug
        /* ret = recv(socket_fd, mark_recv, sizeof(mark_recv), 0);
        if(ret < 0){
            printf("Recv mark_recv failed! : %d, %d\n", mark_recv[0], mark_recv[1]);
            fprintf(error_fp, "\nRecv mark_recv failed! : %d, %d\n", mark_recv[0], mark_recv[1]);
            exit(1);
        }
        board_num = mark_recv[0];
        end_location = mark_recv[1]; */
        recv_count++;
        sem_post(sem + 1);
        printf("debug: recv data success: %d\n", recv_count);
    }
}

int get_timestamp(long long *m_valid, LOCA_TIME *channel_0, int count_0, LOCA_TIME *channel_7, int count_7){
    //long long *m_valid = (long long *)malloc(sizeof(long long) * 1250);
    memset(m_valid, 0, sizeof(m_valid));

    int valid_num = 0;
    int m = 0;
    int i = 0;

    for(; i < count_0;){
        if(abs((channel_0 + i)->m_timestamp - (channel_7 + m)->m_timestamp) < DELAY_MAX){
            *(m_valid + valid_num) = channel_0->m_timestamp;
            valid_num++;
        }
        else if (((channel_0 + i)->m_timestamp - (channel_7 + m)->m_timestamp) > 0){
            m++;
            continue;
        }else{
            i++;
            continue;
        }
    }
    return valid_num;
}

void *example_analys(void *example_me){
    EXAMPLE_B *ex_me = (EXAMPLE_B *)example_me;

    int adc_count[8] = {0};
    int valid_count[8] = {0};
    int ret;
    unsigned int value;
    int _channel;
    int valid_num;
    LOCA_TIME list_adc[8][1250];
    int loca, start_loca;
    memset(list_adc, 0, sizeof(list_adc));

    for(int i = 0; i < ex_me->m_board_num; i++){
        start_loca = ex_me->start_address + i * 1024;
        loca = start_loca + find_board_head(recved_pack + start_loca, 1);
        for (; loca - start_loca < 1024;)
        {
            ret = find_adc_head(recved_pack + loca, 0);
            if(ret == 100){
                break;
            }
            loca += ret;
            _channel = bit_head_read(recved_pack + loca, 'c');
            (list_adc[_channel] + adc_count[_channel])->m_location  = loca;
            (list_adc[_channel] + adc_count[_channel])->m_channel   = _channel;
            (list_adc[_channel] + adc_count[_channel])->m_length    = bit_head_read(recved_pack + loca, 'l');
            (list_adc[_channel] + adc_count[_channel])->m_timestamp = bit_time_read(recved_pack + loca);
            loca += (list_adc[_channel] + adc_count[_channel])->m_length;
            adc_count[_channel]++;
        }
    }
    if(ex_me->m_mark < 2){
        loca = *(ex_me->start_address + ex_me->m_board_num);
        for(int k = 0; k < 16; k++){
            ret = find_adc_head(recved_pack + loca, 0);
            if(ret == 100){
                break;
            }
            loca += ret;
            _channel = bit_head_read(recved_pack + loca, 'c');
            (list_adc[_channel] + adc_count[_channel])->m_location = loca;
            (list_adc[_channel] + adc_count[_channel])->m_channel = _channel;
            (list_adc[_channel] + adc_count[_channel])->m_length = bit_head_read(recved_pack + loca, 'l');
            (list_adc[_channel] + adc_count[_channel])->m_timestamp = bit_time_read(recved_pack + loca);
            loca += (list_adc[_channel] + adc_count[_channel])->m_length;
            adc_count[_channel]++;
        }
    }
    loca = 0;
    long long *valid_time = (long long *)malloc(sizeof(long long) * 1250);
    if(channel_num > 4){
        valid_num = get_timestamp(valid_time, list_adc[0], adc_count[0], list_adc[7], adc_count[7]);
    }else{
        valid_num = get_timestamp(valid_time, list_adc[min_channel], adc_count[min_channel], list_adc[min_channel], adc_count[min_channel]);
    }
    for(int i = 0; i < valid_num; i++){
        for(_channel = min_channel; _channel < min_channel + channel_num; _channel++){
            for(; valid_count[_channel] < adc_count[_channel];){
                if(abs((list_adc[_channel] + valid_count[_channel])->m_timestamp - *(valid_time + i)) < DELAY_MAX){
                    break;
                }else
                {
                    valid_count[_channel]++;
                }
            }
            if (valid_count[_channel] = adc_count[_channel])
            {
                valid_count[_channel] = 0;
                break;
            }
        }
        if (_channel == min_channel + channel_num - 1)
        {
            if(log_save == NULL || valid_example_count == 0xffff){
                if(log_save == NULL){
                    log_save = open_savelog(file_count);
                }else{
                    fclose(log_save);
                    file_count++;
                    log_save = open_savelog(file_count);
                }
            }
            value = valid_example_count << 16;
            for(int c = 0; c < 8; c++){
                loca += (list_adc[c] + valid_count[c])->m_length + 1;
            }
            value = value | loca;
            fwrite(&value, 4, 1, log_save);
            for(int c = 0; c < 8; c++){
                fwrite(recved_pack + (list_adc[c] + valid_count[c])->m_location, 4, (list_adc[c] + valid_count[c])->m_length + 1, log_save);
            }
            fflush(log_save);
        }
    }
    free(valid_time);
}

void *data_analys(){
    int ret;
    int loca;
    int board_location[200] = {0};
    int recv_num;
    int start_num, end_num;
    int start_address, end_address;

    pthread_attr_t attr;
    pthread_t ana_ptd[3];
    EXAMPLE_B *_me;

    while(work != -1){
        board_num = BOARD_NUM;
        sem_wait(sem + 1);
        printf("debug: data_analys start\n");

        /*for(int i = 0; i < board_num; i++){
            ret = find_board_head(recved_pack + board_location[i], 0);
            if(ret == 100){
                board_location[i] = 0;
                board_num = i;
                break;
            }
            board_location[i] += ret;
            board_location[i + 1] = board_location[i] + 1024;
        } 
        loca = board_num / 3 + 1;*/

        ret = find_board_head(recved_pack + BOARD_NUM * 1024 + 1024, 0);
        if (ret < 100)
        {
            for (; board_num < BOARD_NUM + 10; board_num++)
            {
                ret = find_board_head(recved_pack + board_num * 1024 + 1024, 0);
                if (ret == 100)
                {
                    break;
                }
            }
        }
        recv_num = board_num / client_num + 1;

        for(int i = 0; i < 3; i++){
            ret = pthread_attr_init(&attr); //初始化线程属性变量,成功返回0,失败-1
            if (ret < 0)
            {
                perror("Init attr fail");
                exit(1);
            }
            cpu_set_t cpusetinfo;
            CPU_ZERO(&cpusetinfo);
            CPU_SET(4 - i, &cpusetinfo); //将core1加入到cpu集中,同理可以将其他的cpu加入

            ret = pthread_attr_setaffinity_np(&attr, sizeof(cpusetinfo), &cpusetinfo);
            if (ret < 0)
            {
                perror("Core set fail");
                exit(1);
            }

            start_num = i * recv_num;
            start_address = start_num * 1024 + find_board_head(recved_pack + start_num * 1024, 1);

            _me = (EXAMPLE_B *)malloc(sizeof(EXAMPLE_B));
            _me->start_address = start_address;
            if (i == 2){
                _me->m_board_num = board_num - 2 * recv_num;
            }else{
                _me->m_board_num = recv_num;
            }
            _me->m_mark = i;

            pthread_create(ana_ptd + i, &attr, example_analys, _me);
            pthread_attr_destroy(&attr);
        }

        for(int i = 0; i < 3; i++){
            pthread_join(ana_ptd[i], 0);
        }
        free(_me);
        memset(recved_pack, 0, sizeof(recved_pack));
        sem_post(sem + 0);
    }
}

int main(){
    error_fp = open_error_log();
    fprintf(error_fp, "This is recv.c error.log!\n");
    char sig;
    while (sig != 'y')
    {
        printf("Port = %d\nIP = %s\nPress'y' to continue...\n", port, ip);
        sig = getchar();
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

    socket_create();
    printf("This is the %d client!\n", recv_id);
    fprintf(error_fp, "This is the %d client!\n", recv_id);
    ptd_create(&recv_ptd, -1, recv_func);
    ptd_create(&ana_ptd, -1, data_analys);

    while(sig != 'o'){
        sig = getchar();
        switch (sig)
        {
       
        default:
            break;
        }
    }

    close(socket_fd);
    close(connect_fd);
    fclose(error_fp);
    fclose(log_save);
    return 0;
}