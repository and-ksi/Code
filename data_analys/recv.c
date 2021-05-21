#include "ana.h"

//功能控制
int work;
FILE *error_fp; //错误报告

//socket need
char ip[32] = "192.168.3.4";
int socket_fd, connect_fd;
int port = 10000;
int recv_id;
unsigned int recved_pack[PACK_SIZE];

//计数
int recv_count;
int end_location;
int board_num;

//socket create functon, need global identifier:
//socket_fd IP port connect_fd
void socket_create()
{
    int on = 1;
    int ret;
    char buf[32] = {0};

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
    ret = recv(socket_fd, buf, 32, 0);
    if(ret < 0){
        fprintf(error_fp, "\nRecv recv_id failed: buf[%s]\n", buf);
        printf("Recv recv_id failed!\n");
        fclose(error_fp);
        exit(1);
    }
    recv_id = atoi(buf);
    memset(buf, 0, 32);
    sprintf(buf, "%s", "ok");
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
        ret = recv(socket_fd, recved_pack, sizeof(recved_pack), 0);
        if(ret < 0){
            printf("Recv failed! recv_count: %d\n", recv_count);
            fprintf("\nRecv failed! recv_count: %d\n", recv_count);
            fprintf(error_fp, recved_pack);
            fclose(error_fp);
            exit(1);
        }
        ret = recv(socket_fd, mark_recv, sizeof(mark_recv), 0);
        if(ret < 0){
            printf("Recv mark_recv failed! : %d, %d\n", mark_recv[0], mark_recv[1]);
            fprintf("\nRecv mark_recv failed! : %d, %d\n", mark_recv[0], mark_recv[1]);
            exit(1);
        }
        board_num = mark_recv[0];
        end_location = mark_recv[1];
        recv_count++;
    }
}

void *data_analys(){
    int ret;
    int adc_loca, loca;
    int board_location[200] = {0};
    int adc_location[100] = {0};
    int m, n;
    int defrence;
    struct location_timestamp loc_t[8][10];//[m][n]

    int _channel;

    while(work != -1){
        for(int i = 0; i < board_num; i++){
            ret = find_board_head(recved_pack + board_location[i], 0);
            if(ret == 100){
                board_location[i] = 0;
                break;
                /* printf("Find board_head failed! : the %dth board_head.\n", i);
                fprintf(error_fp, "\nFind board_head failed! : the %dth board_head.\n", i);
                write_error_log(error_fp, recved_pack + board_location[i - 1] + 1024);
                exit(1); */
            }
            board_location[i] += ret;
            board_location[i + 1] = board_location[i] + 1024;


            for(int k = 0; k < 1025;){

            }
        }
    }
}