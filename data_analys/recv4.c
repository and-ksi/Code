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
int recv_num;

//socket need
char ip[32] = "192.168.3.4";
int socket_fd, connect_fd;
int port = 10000;
int recv_id;
unsigned int recved_pack[PACK_SIZE];

//计数
int recv_count;
int valid_example_count = 0;
int save_count;

static sem_t sem[5];
pthread_mutex_t mute;
LOCA_TIME list_adc[8][16000];
int adc_count[8] = {0};

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
        while (work > 0 && work < 15);
        memset(recved_pack, 0, sizeof(recved_pack));
        memset(adc_count, 0, sizeof(adc_count));
        printf("debug: start wait for recving data\n");
        for (recved_ll = 0; recved_ll < recv_num * 4;)
        {
            ret = 0;
            while (ret == 0)
            {
                ret = recv(socket_fd, (char *)recved_pack + recved_ll, recv_num * 4 - recved_ll, 0);
            }
            //printf("debug: recv ret = %d\n", ret);
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
        recv_count++;
        sem_post(sem + 1);
        printf("debug: recv data success: %d\n", recv_count);
    }
}

void sort_data()
{
    //debug
    //write_data_error_log(&error_fp, recved_pack, PACK_SIZE, 0);

    memset(list_adc, 0, sizeof(list_adc));

    int loca = 0, start_loca, ret;
    int _channel;
    char buf[50] = {'\0'};
    LOCA_TIME ll_adc;
    //FILE *fp[8];

    for (int board_count = 0; board_count < recv_num / 1024; board_count++)
    {
        ret = find_board_head(recved_pack + board_count * 1024, 0, 0);
        if (ret == 100)
        {
            //printf("can not find board head\n");
            continue;
        }

        loca = board_count * 1024 + ret;
        start_loca = loca;
        //printf("find board head ret = %d, loca = %d     %x\n", ret, loca, recved_pack[loca]);
        for (; loca - start_loca < 1023;)
        {
            if (loca >= PACK_SIZE - 10)
            {
                printf("loca error: loca = %d\n", loca);
                break;
            }
            ret = find_adc_head(recved_pack + loca, 0, 0);
            if (ret == 100)
            {
                //printf("can not find adc head\n");
                break;
            }
            loca += ret;
            if (loca - start_loca >= 1024)
            {
                break;
            }
            //printf("find adc head ret = %d, loca = %d   %x, %x, %x, %x, %x, %x\n", ret, loca, recved_pack[loca - 1], recved_pack[loca], recved_pack[loca + 1], recved_pack[loca + 2], recved_pack[loca + 3], recved_pack[loca + 4]);
            _channel = bit_head_read(recved_pack + loca, 'c');
            //printf("channel = %d, adc_count[%d] = %d\n", _channel + 1, _channel, adc_count[_channel]);
            list_adc[_channel][adc_count[_channel]].m_channel = _channel;
            //printf("0001\n");
            list_adc[_channel][adc_count[_channel]].m_location = loca;
            list_adc[_channel][adc_count[_channel]].m_timestamp = bit_time_read(recved_pack + loca);
            list_adc[_channel][adc_count[_channel]].m_length = bit_head_read(recved_pack + loca, 'l');
            //printf("debug: length = %d\n", list_adc[_channel][adc_count[_channel]].m_length);
            if (list_adc[_channel][adc_count[_channel]].m_length + loca >= PACK_SIZE || list_adc[_channel][adc_count[_channel]].m_length <= 20)
            {
                printf("debug: read channel error!\n");
                break;
            }
            //printf("debug: adc read end!\n");

            loca += list_adc[_channel][adc_count[_channel]].m_length;

            for (int i = adc_count[_channel]; i > 0; i--)
            {
                if (list_adc[_channel][i].m_timestamp < list_adc[_channel][i - 1].m_timestamp)
                {
                    memcpy(&ll_adc, list_adc[_channel] + i, sizeof(LOCA_TIME));
                    memcpy(list_adc[_channel] + i, list_adc[_channel] + i - 1, sizeof(LOCA_TIME));
                    memcpy(list_adc[_channel] + i - 1, &ll_adc, sizeof(LOCA_TIME));
                }
                else
                {
                    break;
                }
            }
            adc_count[_channel]++;
            if (adc_count[_channel] == 16000)
            {
                break;
            }
        }
    }

    for(int c = 0; c < 8; c++){
        printf("channel = %d, adc_count = %d\n", c + 1, adc_count[c]);
    }

    // for(int c = 0; c < 8; c++){
    //     sprintf(buf, "channel_%d.log", c + 1);
    //     fp[c] = fopen(buf, "w+");
    //     if(fp == NULL){
    //         printf("%s open failed!\n", buf);
    //         exit(1);
    //     }
    //     for(int i = 0; i < adc_count[c]; i++){
    //         fprintf(fp[c], "%d :   channel = %d    timestamp = %llx, loca = %d      %x, %x ,%x, %x, %x\n", i, c, list_adc[c][i].m_timestamp, list_adc[c][i].m_location, recved_pack[list_adc[c][i].m_location - 2], recved_pack[list_adc[c][i].m_location - 1], recved_pack[list_adc[c][i].m_location], recved_pack[list_adc[c][i].m_location + 1], recved_pack[list_adc[c][i].m_location + 2]);
    //     }
    //     fclose(fp[c]);
    // }
    //exit(1);
}

void save_data1()
{
    if (log_save == NULL || valid_example_count == 0xffff)
    {
        if (log_save == NULL)
        {
            log_save = open_savelog();
            //fwrite(&file_count, 4, 1, log_save);
        }
        if (valid_example_count == 0xffff)
        {
            fclose(log_save);
            file_count++;
            log_save = open_savelog();
            //fwrite(&file_count, 4, 1, log_save);
        }
    }
    for (int c = 0; c < 8; c++)
    {
        fprintf(log_save, "channel = %d\n", c + 1);
        for (int i = 0; i < adc_count[c]; i++)
        {

            fprintf(log_save, "%d :   channel = %d    timestamp = %llx, loca = %d      %x, %x ,%x, %x, %x, %x, %x\n", i, c + 1, list_adc[c][i].m_timestamp, list_adc[c][i].m_location, recved_pack[list_adc[c][i].m_location - 2], recved_pack[list_adc[c][i].m_location - 1], recved_pack[list_adc[c][i].m_location], recved_pack[list_adc[c][i].m_location + 1], recved_pack[list_adc[c][i].m_location + 2], recved_pack[list_adc[c][i].m_location + 3], recved_pack[list_adc[c][i].m_location + 4]);
        }
        fflush(log_save);
    }
    write_data_error_log(&error_fp, recved_pack, PACK_SIZE, 0);
    fclose(log_save);
    fclose(error_fp);
    error_fp = open_error_log();
    fprintf(error_fp, "the next error log\n");
    log_save = NULL;
}

void save_data()
{
    if (log_save == NULL || valid_example_count == 0xffff)
    {
        if (log_save == NULL)
        {
            log_save = open_savelog();
            if(work > 10){
                fwrite(&file_count, 4, 1, log_save);
            }
        }
        if (valid_example_count == 0xffff)
        {
            fclose(log_save);
            file_count++;
            log_save = open_savelog();
            if (work > 10){
                fwrite(&file_count, 4, 1, log_save);
            }
        }
    }

    int ret, addr[8] = {0}, _channel, defference, value, length;

    for (addr[7] = 0; addr[7] < adc_count[7]; addr[7]++)
    {
        _channel = 6;
        ret = get_latest_data(list_adc[_channel] + addr[_channel], (list_adc[7] + addr[7])->m_timestamp, adc_count[_channel] - addr[_channel]);
        //printf("debug: 0 channel = %d, ret = %d\n", _channel, ret);
        if (ret >= 0)
        {
            addr[6] += ret;

            defference = abs(list_adc[_channel][addr[_channel]].m_timestamp - list_adc[7][addr[7]].m_timestamp);
            //printf("debug: 0 defference = %d\n", defference);
            if (defference >= PRO_PERIOD_CYCLE)
            {
                addr[6] -= ret;
                continue;
            }
            for (_channel = 5; _channel > 3; _channel--)
            {

                ret = get_latest_data(list_adc[_channel] + addr[_channel], (list_adc[7] + addr[7])->m_timestamp, adc_count[_channel] - addr[_channel]);
                //printf("debug: 1 channel = %d, ret = %d\n", _channel, ret);
                if (ret >= 0)
                {
                    addr[_channel] += ret;
                    if (abs(list_adc[_channel][addr[_channel]].m_timestamp - list_adc[7][addr[7]].m_timestamp) > defference + 5)
                    {
                        addr[_channel] -= ret;
                        break;
                    }
                }else{
                    _channel = -2;
                    break;
                }
            }
            if(_channel == -2){
                continue;
            }
            ret = get_latest_data(list_adc[_channel] + addr[_channel], (list_adc[7] + addr[7])->m_timestamp, adc_count[_channel] - addr[_channel]);
            //printf("debug: 1 channel = %d, ret = %d\n", _channel, ret);
            if (ret >= 0)
            {
                addr[_channel] += ret;
                defference = abs(list_adc[_channel][addr[_channel]].m_timestamp - list_adc[7][addr[7]].m_timestamp);
                //printf("debug: 2 defference = %d\n", defference);
                if (defference >= PRO_PERIOD_CYCLE)
                {
                    addr[_channel] -= ret;
                    continue;
                }
                for (_channel = 3; _channel >= 0; _channel--)
                {
                    ret = get_latest_data(list_adc[_channel] + addr[_channel], (list_adc[7] + addr[7])->m_timestamp, adc_count[_channel] - addr[_channel]);
                    //printf("debug: 2 channel = %d, ret = %d\n", _channel, ret);
                    if (ret >= 0)
                    {
                        addr[_channel] += ret;
                        if (abs(list_adc[_channel][addr[_channel]].m_timestamp - list_adc[7][addr[7]].m_timestamp) > defference + 5)
                        {
                            addr[_channel] -= ret;
                            _channel = -2;
                            break;
                        }
                    }else{
                        _channel = -2;
                        break;
                    }
                }
            }
        }
        //printf("debug: end    _channel = %d\n", _channel);
        if (_channel == -1)
        {
            if(work > 10){
                length = 0;
                value = valid_example_count << 16;
                for (int c = 0; c < 8; c++)
                {
                    length += (list_adc[c] + addr[c])->m_length;
                }
                value = length | value;
                fwrite(&value, 4, 1, log_save);
                for (int c = 0; c < 8; c++)
                {
                    fwrite(recved_pack + (list_adc[c] + addr[c])->m_location, 4, (list_adc[c] + addr[c])->m_length + 1, log_save);
                }
            }else{
                fprintf(log_save, "Example %d\n", valid_example_count);
                for (int c = 0; c < 8; c++)
                {
                    fprintf(log_save, "channel[%d]  timestamp = %lld\n", c, (list_adc[c] + addr[c])->m_timestamp);
                }
            }
            valid_example_count++;
            fflush(log_save);
        }
    }
    fclose(log_save);
    log_save = NULL;
}

void *data_analys(int id)
{
    int ret;

    while (work != -1)
    {
        board_num = BOARD_NUM;
        sem_wait(sem + 1);

        sort_data();

        //save_data1();

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

    int speed[2];

    work = 0;
    pthread_t recv_ptd, ana_ptd, speedt;
    sem_init(sem + 0, 0, 1);
    sem_init(sem + 1, 0, 0);
    sem_init(sem + 2, 0, 0);
    sem_init(sem + 3, 0, 0);
    sem_init(sem + 4, 0, 0);
    log_save = NULL;

    SPEED_T sp_t;
    sp_t.count = (long *)&recv_count;
    sp_t.size = RX_SIZE / 1024 / 2;
    sp_t.speed = speed;
    sp_t.m_fp = &error_fp;

    socket_create();
    printf("This is the %d client!\n", recv_id);
    fprintf(error_fp, "This is the %d client!\n", recv_id);
    ptd_create(&recv_ptd, 0, recv_func, 0, 0);
    ptd_create(&ana_ptd, 0, data_analys, 0, 0);
    //pthread_create(&speedt, 0, speed_test, (void *)&sp_t);

    while (sig != 'o')
    {
        sig = getchar();
        switch (sig)
        {
        case 'w':
            work = 1;
            sleep(1);
            save_data1();
            break;

        case 'l':
            work = 1;
            sleep(1);
            save_data();
            break;

        case 'e':
            work = 11;
            sleep(1);
            save_data();
            break;

        case 'p':
            work = 1;
            break;

        case 'c':
            work = 0;
            break;

        case 't':
            if(work == 1){

            }
            break;
        }
    }

    close(socket_fd);
    close(connect_fd);
    fclose(error_fp);
    fclose(log_save);
    exit(1);
}

int wrete_time(){
    FILE *fp = fopen("log/time.log", "w+");
    if(fp == NULL){
        fprintf(fp, "open time.log failed!\n");
        return 0;
    }
    for(int i = 0; i < 8; i++){
        for(int c = 0; c < 8; c++){
            fprintf(fp, "%d     %d", i, c);
        }
    }
}