#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <memory.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define PACK_SIZE (8 * 1024)

char IP[32] = "192.168.3.1";
char pack_recved[PACK_SIZE];
int port = 10000;
int recv_alarm, recv_count;

void *pack_recv(){
    int socket_fd;
    int connect_fd;
    int ret;
    recv_count = 0;

    struct sockaddr_in serveraddr = {0};
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = port;
    inet_pton(AF_INET, IP, &serveraddr.sin_addr.s_addr);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        printf("Socket fail!\n");
        exit(1);
    }
    connect_fd = connect(socket_fd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (connect_fd < 0)
    {
        printf("Connect fail!\n");
        exit(1);
    }
    memset(&pack_recved, '\0', PACK_SIZE);
    recv_alarm = 0;

    while(1){
        while(recv_alarm == 1)
        ret = recv(socket_fd, pack_recved, PACK_SIZE, 0);
        if (ret < 0)
        {
            printf("Recv fail!\n");
            break;
        }
        recv_count++;
        recv_alarm = 1;
    }
    exit(1);
}

void *data_read(){



    return NULL;
}