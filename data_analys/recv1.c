//printf("debug: \n");
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
    for(on = 0; on < sizeof(recv_info);){
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
    if(recv_id == client_num - 1){
        recv_num = (BOARD_NUM - (client_num - 1) * ret) * 1024;
    }else{
        recv_num = ret * 1024;
    }

    while (work != -1)
    {
        sem_wait(sem + 0);
        printf("debug: start wait for recving data\n");
        for(recved_ll = 0; recved_ll < recv_num * 4;){
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

        //write_data_error_log(&error_fp, recved_pack, PACK_SIZE, 0);
        // exit(1);

        //debug
        // write_data_error_log(&error_fp, recved_pack, recv_num, 0);
        // exit(1);

        //debug
        /* ret = recv(socket_fd, mark_recv, sizeof(mark_recv), 0);
        if(ret < 0){
            printf("Recv mark_recv failed! : %d, %d\n", mark_recv[0], mark_recv[1]);
            fprintf(error_fp, "\nRecv mark_recv failed! : %d, %d\n", mark_recv[0], mark_recv[1]);
            exit(1);
        }
        board_num = mark_recv[0];
        end_location = mark_recv[1]; */
        while(work == 1);
        recv_count++;
        sem_post(sem + 1);
        printf("debug: recv data success: %d\n", recv_count);
    }
}

int get_timestamp(long long *m_valid, LOCA_TIME *channel_0, int count_0, LOCA_TIME *channel_7, int count_7)
{
    //long long *m_valid = (long long *)malloc(sizeof(long long) * 1250);
    memset(m_valid, 0, sizeof(m_valid));

    int valid_num = 0;
    int m = 0;
    int i = 0;

    for (; i < count_0;)
    {
        printf("%llx, %llx", (channel_0 + i)->m_timestamp, (channel_7 + m)->m_timestamp);
        if (abs((channel_0 + i)->m_timestamp - (channel_7 + m)->m_timestamp) < DELAY_MAX)
        {
            *(m_valid + valid_num) = channel_0->m_timestamp;
            //printf("debug: \n");
            valid_num++;
        }
        else if (((channel_0 + i)->m_timestamp - (channel_7 + m)->m_timestamp) > 0)
        {
            m++;
            continue;
        }
        else
        {
            i++;
            continue;
        }
    }
    return valid_num;
}

void *example_analys(void *example_me)
{
    EXAMPLE_B *ex_me = (EXAMPLE_B *)example_me;

    int adc_count[8] = {0};
    int valid_count[8] = {0};
    int ret;
    unsigned int value;
    int _channel;
    int valid_num = 0;
    LOCA_TIME list_adc[8][50];
    int loca, start_loca, m, n;
    memset(list_adc, 0, sizeof(list_adc));

    long long valid_time[50] = {0};

    while (work != -1)
    {
        //printf("debug: example analys start\n");
        sem_wait(sem + ex_me->m_mark + 2);
        for (int i = 0; i < ex_me->m_board_num; i++)
        {
            //printf("debug: find 第%d个board>head\n", i);
            start_loca = ex_me->start_address + i * 1024;
            if(start_loca >= PACK_SIZE - 10){
                break;
            }
            //printf("debug: get board0 head ret = %d\n", ret);
            ret = find_board_head(recved_pack + start_loca, 0);

            // printf("debug: ret = %d\n", ret);
            // exit(1);

            if (ret == 100)
            {
                i++;
                //debug
                printf("未读取到board_head, 第%d个board\n", i);
                exit(1);

                break;
            }
            loca = start_loca = start_loca + ret;

            //printf("debug: loca: %d, start_loca: %d\n", loca, start_loca);
            memset(adc_count, 0, sizeof(adc_count));
            for (; loca - start_loca < 1024;)
            {
                if (loca >= PACK_SIZE - 10)
                {
                    i++;
                    break;
                }
                
                ret = find_adc_head(recved_pack + loca, 0);
                //printf("debug: get adc0 head ret = %d\n", ret);

                // write_data_error_log(&error_fp, recved_pack + loca, 100, 0);
                // printf("debug: ret = %d\n", ret);
                // exit(1);

                if (ret == 100 || loca + ret - start_loca >= 1024)
                {
                    if(start_loca + 1024 >= PACK_SIZE - 10){
                        i++;
                        break;
                    }
                    
                    ret = find_board_head(recved_pack + start_loca + 1024, 0);
                    //printf("debug: get board1 head ret = %d\n", ret);
                    if (ret == 100)
                    {
                        i++;
                        break;
                    }
                    loca = ret + start_loca + 1024;
                    for (int i = 0; i < 12; i++)
                    {
                        if (loca >= PACK_SIZE - 10)
                        {
                            i++;
                            break;
                        }
                        
                        ret = find_adc_head(recved_pack + loca, 0);
                        //printf("debug: get adc1 head ret = %d, %x\n", ret, *(recved_pack + loca + ret));
                        if (ret == 100)
                        {
                            i++;
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
                        //printf("debug: timestamp = %llx", (list_adc[_channel] + adc_count[_channel])-> m_timestamp);
                        //printf("debug: adc_count[%d] : %d\n", _channel, adc_count[_channel]);
                    }
                }
                else
                {
                    loca += ret;
                    _channel = bit_head_read(recved_pack + loca, 'c');
                    (list_adc[_channel] + adc_count[_channel])->m_location = loca;
                    (list_adc[_channel] + adc_count[_channel])->m_channel = _channel;
                    (list_adc[_channel] + adc_count[_channel])->m_length = bit_head_read(recved_pack + loca, 'l');
                    (list_adc[_channel] + adc_count[_channel])->m_timestamp = bit_time_read(recved_pack + loca);
                    loca += (list_adc[_channel] + adc_count[_channel])->m_length;
                    adc_count[_channel]++;
                    //printf("debug: get adc %d", adc_count[_channel]);
                }
            }
            //printf("debug: 读取第%d个BOARD  ADC finished, analys adc_head\n", i);

            //debug
            // printf("debug: adc_count: %d\n", adc_count[1]);
            // write_data_error_log(&error_fp, recved_pack + start_loca, 50, 0);
            // fprintf(error_fp, "\n");
            // for(int m = 0; m < 8; m++){
            //     for(int n = 0; n < adc_count[m]; n++){
            //         fprintf(error_fp, "adc channel: %d\nlocation: %d\nlength: %d\ntimestamp:%lld\n\n", (list_adc[m] + n)->m_channel,
            //                 (list_adc[m] + n)->m_location, (list_adc[m] + n)->m_length, (list_adc[m] + n)->m_timestamp);
            //     }
            // }
            // fprintf(error_fp, "\n");
            // exit(1);
            //printf("adc_count[0]: %d, adc_count[4]: %d, adc_count[7]: %d\n", adc_count[0], adc_count[4], adc_count[7]);
            //valid_time = (long long *)malloc(sizeof(long long) * 1280);
            if (channel_num > 4)
            {
                // valid_num = get_timestamp(valid_time, list_adc[0], adc_count[0], list_adc[7], adc_count[7]);
                if(adc_count[0] > adc_count[7]){
                    adc_count[0] = adc_count [7];
                }
                n = 0;
                valid_num = 0;
                for(m = 0; m < adc_count[0];){
                    if(abs((list_adc[0] + m)->m_timestamp - (list_adc[7] + n)->m_timestamp) < DELAY_MAX){
                        *(valid_time + valid_num) = (list_adc[0] + m)->m_timestamp;
                        valid_num++;
                        m++;
                        n++;
                        if (n == adc_count[7])
                        {
                            break;
                        }
                    }
                    else if ((list_adc[0] + m)->m_timestamp > (list_adc[7] + n)->m_timestamp){
                        m++;
                    }else
                    {
                        n++;
                        if(n == adc_count[7]){
                            break;
                        }
                    }
                }
                
                //debug
                // for(int y = 0; y < valid_num; y++){
                //     printf("debug: %dtimestamp = %lld\n", y, valid_time[y]);
                // }

                //printf("debug: get_timestarm finished\n");
                //exit(1);
            }
            else
            {
                valid_num = get_timestamp(valid_time, list_adc[min_channel], adc_count[min_channel], list_adc[min_channel], adc_count[min_channel]);
            }

            for(int c = 0; c < min_channel + channel_num; c++){
                if(adc_count[c] < valid_num){
                    valid_num = adc_count[c];
                }
            }

            // debug
            // write_data_error_log(&error_fp, (unsigned int *)valid_time, 100, 0);
            // exit(1);
            memset(valid_count, 0, sizeof(valid_count));
            for (int i = 0; i < valid_num; i++)
            {
                for (_channel = min_channel; _channel < min_channel + channel_num; _channel++)
                {
                    for (; valid_count[_channel] < adc_count[_channel];)
                    {
                        if (abs((list_adc[_channel] + valid_count[_channel])->m_timestamp - *(valid_time + i)) < DELAY_MAX)
                        {
                            break;
                        }
                        else
                        {
                            valid_count[_channel]++;
                        }
                    }
                    if (valid_count[_channel] == adc_count[_channel])
                    {
                        valid_count[_channel] = 0;
                        break;
                    }
                }
                if (_channel == min_channel + channel_num)
                {
                    //printf("debug: start save valid data\n");
                    if (log_save == NULL || valid_example_count == 0xffff)
                    {
                        pthread_mutex_lock(&mute);
                        if (log_save == NULL)
                        {
                            log_save = (FILE *)open_savelog(file_count);
                        }
                        else
                        {
                            fclose(log_save);
                            file_count++;
                            log_save = open_savelog(file_count);
                        }
                        pthread_mutex_unlock(&mute);
                    }
                    loca = 0;
                    for (int c = min_channel; c < min_channel + channel_num; c++)
                    {
                        loca += (list_adc[c] + valid_count[c])->m_length + 1;
                    }
                    pthread_mutex_lock(&mute);
                    value = valid_example_count << 16;
                    valid_example_count++;
                    value = value | loca;
                    
                    fwrite(&value, 4, 1, log_save);
                    for (int c = min_channel; c < min_channel + channel_num; c++)
                    {
                        fwrite(recved_pack + (list_adc[c] + valid_count[c])->m_location, 4, ((list_adc[c] + valid_count[c])->m_length + 1), log_save);
                        //printf("debug: write savelog success!\n");
                    }
                    fflush(log_save);
                    pthread_mutex_unlock(&mute);
                    //printf("debug: save valid data finished\n");
                }
            }
            //free(valid_time);
        }
        //printf("debug: example analys finished\n");
        sem_post(sem + 1);
    }
}

void *data_analys()
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
        ptd_create(ana_ptd + i, i + 1, example_analys, _me + i, 0);
    }

    while (work != -1)
    {
        board_num = BOARD_NUM;
        sem_wait(sem + 1);
        //printf("debug: data_analys start\n");

        ret = find_board_head(recved_pack + PACK_SIZE - 1024, 0);
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
        for(int i = 0; i < 1; i++){

            start_num = i * recv_num;

            start_address = start_num * 1024 + find_board_head(recved_pack + start_num * 1024, 0);
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