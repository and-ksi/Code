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

#define DEVICE_NAME_H2C "/dev/xdma0_h2c_0"
#define DEVICE_NAME_C2H "/dev/xdma0_c2h_0"
#define DEVICE_NAME_REG "/dev/xdma0_bypass"

#define MAP_SIZE        (8*1024)//8bit
#define IMG_RAM_POS     (0*1024*1024)
#define CPU_CORE        (4)//CPU核心数量,使用顺序从数值最大的核心开始分配,留下第一个核心不分配

int c2h_fd ;
int h2c_fd ;
int control_fd;
void *control_base;
int interrupt_fd;

int pData[8*1024];
int work;

int port = 10000;
int acfd;

struct sockaddr_in clientaddr = {0};
pthread_t ptd[10];

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
    read(fd,&val,4);
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
        read_event(interrupt_fd);  //获取用户中断
        read(c2h_fd, pData, 8*1024);
        write_control(control_base,0x0000,0xFFFFFFFF);
    }
    pthread_exit(0);
}

//根据CPU创建和分配线程
void ptd_create(int *arg, void *functionbody) {
	int ret;

	cpu_set_t cpusetinfo;
	CPU_ZERO(&cpusetinfo);
	CPU_SET((CPU_CORE - 1 - *arg), &cpusetinfo);//将core1加入到cpu集中,同理可以将其他的cpu加入

	pthread_attr_t attr;

	ret = pthread_attr_init(&attr);//初始化线程属性变量,成功返回0,失败-1
	if(ret < 0) {
		perror("Init attr fail");
		exit(1);
	}
	/* ret = pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);//PTHREAD_SCOPE_SYSTEM绑定;PTHREAD_SCOPE_PROCESS非绑定
	if(ret < 0) {
		perror("Setscope fail");
		exit(1);
	} */
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//线程分离属性:PTHREAD_CREATE_JOINABLE（非分离）
	if(ret < 0) {
		perror("Detached fail");
		exit(1);
	}
	ret = pthread_attr_setaffinity_np(&attr, sizeof(cpusetinfo), &cpusetinfo);
	if(ret < 0) {
		perror("Core set fail");
		exit(1);
	}
	
	pthread_create(&ptd[*arg], &attr, (void *(*)(void *))functionbody, NULL);
	
	pthread_attr_destroy(&attr);//销除线程属性

    *arg = -1;
}

//socket和线程创建函数
void socket_ptd_create(){
    int socket_fd;
    int ret;
    int id;
    socklen_t len;

    struct sockaddr_in localaddr = {0};
    localaddr.sin_family = AF_INET;
    localaddr.sin_port = htons(port);
    localaddr.sin_addr.s_addr = INADDR_ANY;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        printf("Socket fail!\n");
		exit(1);
    }
    ret = bind(socket_fd, (struct sockaddr *)&localaddr, sizeof(localaddr));
	if(ret < 0) {
		printf("Bind fail!\n");
		exit(1);
	}
    ret = listen(socket_fd, 10);
	if(ret < 0) {
		printf("Listen fail!\n");
		exit(1);
	}

    for(int i = 0; i < CPU_CORE - 1; i++) {
		printf("等待连接中......");
		acfd = accept(socket_fd, (struct sockaddr *)&clientaddr, &len);
		id = i;
        ptd_create(&id, NULL);//这里根据需要更改
        printf("接到地址%s 的连接")
		while(1) {
			if(id < 0) {
				break;
			}
		}	
	}
}

int main() {

}