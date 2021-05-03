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


#define DEVICE_NAME_H2C_0 "/dev/xdma0_h2c_0"
#define DEVICE_NAME_C2H_0 "/dev/xdma0_c2h_0"
#define DEVICE_NAME_H2C_1 "/dev/xdma0_h2c_1"
#define DEVICE_NAME_C2H_1 "/dev/xdma0_c2h_1"

#define MAP_SIZE        (0x7FFFFFFF)//8bit 2GB
#define MAP_BYPASS_SIZE (4*1024)
#define IMG_RAM_POS     (0)
#define RX_SIZE         (50*1024*1024)//1byte 8bit


pthread_t event_thread;
pthread_t rx_thread_0;
pthread_t rx_thread_1;
pthread_t speed_ptd;

int work;

int c2h_fd_0 ;
int c2h_fd_1 ;

int h2c_fd ;
int control_fd;
void *control_base;
int interrupt_fd;

int read_end_addr;

int pData_0[RX_SIZE/8];
int pData_1[RX_SIZE/8];

int cnt;

static sem_t int_sem_rx[2];


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
    //printf("Reading\n"); 
    read(fd,&val,4);
    //printf("event=%x\n",val);
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
    interrupt_fd = open_event("/dev/xdma0_events_0");    //打开用户中断
    while(work==1)
    {
    	//printf("Start read interrupt !\n");     
        read_event(interrupt_fd);  //获取用户中断
        read_end_addr = read_control(control_base,0x0008);//read interrupt reg
        write_control(control_base,0x0000,0xFFFFFFFF);//ack interrupt
        sem_post(&int_sem_rx[0]);//release signal
        sem_post(&int_sem_rx[1]);//release signal
        //printf("read_end_addr = %x \n",read_end_addr);
        //lseek(c2h_fd, read_end_addr, SEEK_SET);
        //read(c2h_fd, pData_0, 32*1024);
        //cnt = cnt +1;
        //printf("cnt = %d\n",cnt);
        //printf("Stop read!\n"); 
    }
    pthread_exit(0);  
}

void *speed_test() 
{
    struct timeval systime;
    long int time1, time2, starttime;
    int cnt1 = 0;
    int speed1, speed2;
    gettimeofday(&systime, 0);
    starttime = time1 = time2 = systime.tv_sec;
    while(1) 
    {
        gettimeofday(&systime, 0);
        time1 = systime.tv_sec;
        if(time1 - time2 >= 1) 
        {
            speed1 = (50 * (cnt - cnt1));
            speed2 = ((50 * cnt)/(time1 - starttime));
            printf("%ld s, speed in one sec:%d MB/s, speed average:%d MB/s\n",
             time1 - starttime, speed1, speed2);
            time2 = time1;
            cnt1 = cnt;
        }
    }
    pthread_exit(0);
}

void *rx_process_0( )
{
    while(work == 1)
    {
        sem_wait(&int_sem_rx[0]);
        printf("read_end_addr = %x \n",read_end_addr);
        lseek(c2h_fd_0, (read_end_addr - 50*1024*1024), SEEK_SET);
        read(c2h_fd_0, pData_0, 25*1024*1024);//10MB
        cnt = cnt +1;
        printf("cnt = %d\n",cnt);
    }
    pthread_exit(0);
}

void *rx_process_1( )
{
    while(work == 1)
    {
        sem_wait(&int_sem_rx[1]);
        printf("read_end_addr = %x \n",read_end_addr);
        lseek(c2h_fd_1, (read_end_addr - 25*1024*1024), SEEK_SET);
        read(c2h_fd_1, pData_1, 25*1024*1024);//10MB
        //cnt = cnt +1;
        //printf("cnt = %d\n",cnt);
    }
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    char  inp ;
    unsigned int pos = 0;
    
    unsigned int cnontrol_3;
    
    sem_init(&int_sem_rx[0], 0, 0);
    sem_init(&int_sem_rx[1], 0, 0);
    
    control_fd = open_control("/dev/xdma0_bypass");//打开bypass字符设备
    control_base = mmap_control(control_fd,MAP_BYPASS_SIZE);//获取bypass映射的内存地址
    
    c2h_fd_0 = open(DEVICE_NAME_C2H_0, O_RDONLY | O_NONBLOCK);//打开pcie c2h_0设备
    if(c2h_fd_0 == -1)
    {
        printf("PCIe c2h_0 device open failed!\n");
    }
    else
        printf("PCIe c2h_0 device open successful!\n"); 

    mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, c2h_fd_0, 0);
   
    printf("c2h_0 device Memory map successful!\n");
    
    c2h_fd_1 = open(DEVICE_NAME_C2H_1, O_RDONLY | O_NONBLOCK);//打开pcie c2h_1设备
    if(c2h_fd_1 == -1)
    {
        printf("PCIe c2h_1 device open failed!\n");
    }
    else
        printf("PCIe c2h_1 device open successful!\n"); 

    mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, c2h_fd_1, 0);
   
    printf("c2h_1 device Memory map successful!\n");
    
    h2c_fd = open(DEVICE_NAME_H2C_0, O_WRONLY | O_NONBLOCK);//打开pcie  h2c设备
    if(h2c_fd == -1)
    {
        printf("PCIe h2c device open failed!\n");
    }
    else
        printf("PCIe h2c device open successful!\n"); 

    mmap(0, MAP_SIZE, PROT_WRITE, MAP_SHARED, h2c_fd, 0);
   
    printf("h2c device Memory map successful!\n");
    
    work = 1;
    
    pthread_create(&event_thread, NULL, event_process, NULL);
    pthread_create(&rx_thread_0, NULL, rx_process_0, NULL);
    pthread_create(&rx_thread_1, NULL, rx_process_1, NULL);
    pthread_create(&speed_ptd, NULL, speed_test, NULL);
    
    while(inp!='o')
    {
    inp = getchar();
    	switch(inp)
    	{
    	case'w':
    	    lseek(c2h_fd_0,0, SEEK_SET);
    	    read(c2h_fd_0, pData_0, 4*1024);
            printf("Read data successful!\n");

            for(int i=0;i<1024;i++)
            {
                printf("The data of address %d is %x \n", i,pData_0[i]);
            }
        break;
            
        case'e':
    	    cnt = 0;
        break;
        
        case'r':
    	    write_control(control_base,0x0000,0xFFFFFFFF);
        break;
        
        case't':
    	    write_control(control_base,0x0004,0xFFFFFFFF);
        break;
        
        case'y':
    	    cnontrol_3 = read_control(control_base,0x0008);
    	    printf("read_end_addr = %x \n",cnontrol_3);
        break;
        
        case'u':
    	    pos = pos + 1024;
    	    printf("test_read_end_addr = %x \n",pos);
        break;
        
        case'i':
    	    pos = 0;
    	    printf("test_read_end_addr = %x \n",pos);
        break;
        
        case'g':
    	    //memcpy(pData_1, pData_0, sizeof(pData_1));
    	    //FILE *fp;
    	    //fp = fopen("data.log", "w+");
    	    //if(fp == NULL)
    	    //{
    	    //	printf("File open failed!");
    	    //	break;
    	    //}
    	    //fwrite(pData_1, 1, sizeof(pData_1), fp);
    	    //fclose(fp);
    	    //memset(pData_1, '0', sizeof(pData_1));
        break;
    	}
    
    }
    work = 0;
    
    close(c2h_fd_0);
    close(c2h_fd_1);
    close(h2c_fd);
    close(control_fd);
    close(interrupt_fd);

    return 0;
}
