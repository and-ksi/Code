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


#define DEVICE_NAME_H2C "/dev/xdma0_h2c_0"
#define DEVICE_NAME_C2H "/dev/xdma0_c2h_0"
#define DEVICE_NAME_REG "/dev/xdma0_bypass"

#define MAP_SIZE        (8*1024)//8bit
#define IMG_RAM_POS     (0*1024*1024)


pthread_t event_thread;

int work;

int c2h_fd ;
int h2c_fd ;
int control_fd;
void *control_base;
int interrupt_fd;

int pData[8*1024];

int cnt;

pthread_t speed_thread;
int speed1, speed2;
long int starttime, time1, time2;
struct timeval time;


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
/*open bypass devies*/
static int open_control(char *filename)
{
    int fd;
    fd = open(filename, O_RDWR | O_SYNC);
    if(fd == -1)
    {
        printf("open control error\n");
        return -1;
    }
    return fd;
}

//map bypass devies addrs
static void *mmap_control(int fd,long mapsize)
{
    void *vir_addr;
    vir_addr = mmap(0, mapsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    return vir_addr;
}

//write bypass devies
static void write_control(void *base_addr,int offset,uint32_t val)
{
    //uint32_t writeval = htoll(val);
    *((uint32_t *)(base_addr+offset)) = val;
}

//read bypass deives
static uint32_t read_control(void *base_addr,int offset)
{
    uint32_t read_result = *((uint32_t *)(base_addr+offset));
    //read_result = ltohl(read_result);
    return read_result;
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
        read(c2h_fd, pData, 8*1024);
        write_control(control_base,0x0000,0xFFFFFFFF);
        cnt = cnt +1;
        printf("cnt = %d\n",cnt);
        printf("Stop read!\n"); 
    }
    pthread_exit(0);  
}

void *speed_test() {
    gettimeofday(&time, 0);
    starttime = time1 = time2 = time.tv_sec;
    while(1) {
        gettimeofday(&time, 0);
        time1 = time.tv_sec;
        if(time1 - time2 >= 1) {
            speed1 = 4*cnt/1024;
            speed2 = 4*cnt/1024/(time1 - starttime);
            printf("%ld s, speed in one sec:%d MB/s, speed average:%d MB/s",
             time1, speed1, speed2);
            time2 = time1;
        }
    }

}

int main(int argc, char *argv[])
{
    char  inp ;
    
    control_fd = open_control("/dev/xdma0_bypass");//打开bypass字符设备
    control_base = mmap_control(control_fd,MAP_SIZE);//获取bypass映射的内存地址
    
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
    
    pthread_create(&event_thread, NULL, (void *(*)(void *))event_process, NULL);
    pthread_create(&speed_thread, NULL, (void *(*)(void *))speed_test, NULL);
    
    while(inp!='o')
    {
    inp = getchar();
    	switch(inp)
    	{
    	case'w':
    	    //read(c2h_fd, pData, 8*1024);

            printf("Read data successful!\n");

            for(int i=0;i<1024;i++)
            {
                printf("The data of address %d is %x \n", i,pData[i]);
            }
        break;
            
        case'e':
    	    cnt = 0;
        break;
        
        case'r':
    	    write_control(control_base,0x0004,0xFFFFFFFF);
        break;
    	}
    
    }
    work = 0;

    close(c2h_fd);
    close(h2c_fd);
    close(control_fd);
    close(interrupt_fd);

END:
    return 0;
}
