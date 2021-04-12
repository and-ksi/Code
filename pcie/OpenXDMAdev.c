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
#include <pthread.h>

#include <libxdma_api.h>


#define DEVICE_NAME_H2C "/dev/xdma0_h2c_0"
#define DEVICE_NAME_C2H "/dev/xdma0_c2h_0"
#define DEVICE_NAME_REG "/dev/xdma0_bypass"

#define MAP_SIZE        (8*1024)//8bit
#define IMG_RAM_POS     (0*1024*1024)


pthread_t event_thread;

pci_dev xdma_dev;

void *dev_handle;

int work;

int c2h_fd ;
int h2c_fd ;
int interrupt_fd;

int pData[8*1024];


void irq_printf()
{
    printf("IRQ OK!\n");
}

/*开中断*/
int open_event(char *devicename)
{
int fd;
fd=open(devicename,O_RDWR|O_SYNC );
if(fd==-1)
{
    printf("open event error\n");
    return -1;
 } 
printf("open event successful!\n");
return fd;
}

/*获取用户中断*/
int read_event(int fd)
{
int val;
printf("Reading\n"); 
read(fd,&val,4);
printf("event=%x\n",val);
return val;
}

/*中断处理进程*/
void *event_process()
{
    int i;
    int txint_rc;
    interrupt_fd = open_event("/dev/xdma0_events_0");    //打开用户中断
    while(work==1)
    {        
    	printf("Start read!\n");     
        read_event(interrupt_fd);  //获取用户中断
        printf("Stop read!\n"); 
    }
    pthread_exit(0);  
}

int main(int argc, char *argv[])
{
    char  inp ;
    
    dev_handle = xdma_device_open(xmda, xdma_dev,1,2,2);
    
    xdma_user_isr_register(dev_handle,1,irq_printf, NULL);
    
    xdma_user_isr_enable(dev_handle, 1);
    
    c2h_fd = open(DEVICE_NAME_C2H, O_RDONLY | O_NONBLOCK);//打开pcie c2h设备
    if(c2h_fd == -1)
    {
        printf("PCIe c2h device open failed!\n");
        goto END;
    }
    else
        printf("PCIe c2h device open successful!\n"); 

    mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, c2h_fd, 0);
   
    printf("c2h device Memory map successful!\n");
    
    h2c_fd = open(DEVICE_NAME_H2C, O_WRONLY | O_NONBLOCK);//打开pcie  h2c设备
    if(h2c_fd == -1)
    {
        printf("PCIe h2c device open failed!\n");
        goto END;
    }
    else
        printf("PCIe h2c device open successful!\n"); 

    mmap(0, MAP_SIZE, PROT_WRITE, MAP_SHARED, h2c_fd, 0);
   
    printf("h2c device Memory map successful!\n");
    
    work = 1;
    
    pthread_create(&event_thread, NULL, event_process, NULL);
    
    while(inp!='o')
    {
    inp = getchar();
    	switch(inp)
    	{
    	case'w':
    	    read(c2h_fd, pData, 8*1024);

            printf("Read data successful!\n");

            for(int i=0;i<1024;i++)
            {
                printf("The data of address %d is %x \n", i,pData[i]);
            }
            break;
    	}
    
    
    }
    work = 0;

    close(c2h_fd);
    close(h2c_fd);

END:
    return 0;
}
