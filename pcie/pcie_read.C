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
#include <sys/mman.h>
#include <errno.h>


#define DEVICE_NAME_H2C "/dev/xdma0_h2c_0"
#define DEVICE_NAME_C2H "/dev/xdma0_c2h_0"
#define DEVICE_NAME_REG "/dev/xdma0_bypass"

#define MAP_SIZE        (2*1024*1024*1024)//8bit
#define IMG_RAM_POS     (0*1024*1024)

int pcie_fd;
void * pcie_addr;
int pData[1024*1];

int main(int argc, char *argv[])
{
    pcie_fd = open(DEVICE_NAME_C2H, O_RDWR | O_NONBLOCK);//打开pcie字符设备
    if(pcie_fd == -1)
    {
        printf("PCIe device open failed!\n");
        goto END;
    }
    else
        printf("PCIe device open successful!\n"); 

    pcie_addr   = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pcie_fd, 0);
   
    printf("Memory map successful!\n");

    lseek(pcie_fd, IMG_RAM_POS, SEEK_SET);
    read(pcie_fd, pData, 8*1024);

    printf("Read data successful!\n");

    for(int i=0;i<8*1024;i++)
    {
        printf("The data of address %d is %d \n", i,pData[i]);
    }

    close(pcie_fd);

END:
    return 0;
}
