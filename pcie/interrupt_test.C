#include <sys/mman.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(){
    int contrl_fd;
    void *contrl_base;

    contrl_fd = open("/dev/xdma0_bypass", O_RDWR | O_SYNC);
    if(contrl_fd < 0){
        printf("Open contrl failed!");
        return -1;
    }
    contrl_base = mmap(0, mapsize, PROT_READ | PROT_WRITE, MAP_SHARED, contrl_fd, 0);

}