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

#define MAP_SIZE        (32*1024)//8bit
#define BYPASS_MAP_SIZE (4*1024)
#define IMG_RAM_POS     (0*1024*1024)
#define CPU_CORE        (4)//CPU核心数量,使用顺序从数值最大的核心开始分配,留下第一个核心不分配
#define PACK_SIZE       (8*1024)
#define CLIENT_NUM      (7)

int c2h_fd ;
int h2c_fd ;
int control_fd;
void *control_base;
int interrupt_fd;
pthread_t event_thread;

char pData[MAP_SIZE];
int work;

int port = 10000;
int acfd[10];
int ptd_alarm, part_alarm;
int count = 0;

struct sockaddr_in clientaddr[10] = {0};
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
        while(part_alarm == 1){}
        read(c2h_fd, pData, sizeof(pData));
        count++;
        part_alarm = 1;
        write_control(control_base,0x0000,0xFFFFFFFF);
    }
    pthread_exit(0);
}

//打开设备并进行读取
void read_device(){
    char inp;
    control_fd = open_control("/dev/xdma0_bypass");    //打开bypass字符设备
    control_base = mmap_control(control_fd, BYPASS_MAP_SIZE); //获取bypass映射的内存地址

    c2h_fd = open(DEVICE_NAME_C2H, O_RDONLY | O_NONBLOCK); //打开pcie c2h设备
    if (c2h_fd == -1)
    {
        printf("PCIe c2h device open failed!\n");
        exit(1);
    }
    else
        printf("PCIe c2h device open successful!\n");

    mmap(0, MAP_SIZE, PROT_READ, MAP_SHARED, c2h_fd, 0);

    printf("c2h device Memory map successful!\n");

    h2c_fd = open(DEVICE_NAME_H2C, O_WRONLY | O_NONBLOCK); //打开pcie  h2c设备
    if (h2c_fd == -1)
    {
        printf("PCIe h2c device open failed!\n");
        exit(1);
    }
    else
        printf("PCIe h2c device open successful!\n");

    mmap(0, MAP_SIZE, PROT_WRITE, MAP_SHARED, h2c_fd, 0);

    printf("h2c device Memory map successful!\n");

    work = 1;

    pthread_create(&event_thread, NULL, (void *(*)(void *))event_process, NULL);
    while (inp != 'o')
    {
        inp = getchar();
        switch (inp)
        {
        case 'w':
            //read(c2h_fd, pData, 8*1024);

            printf("Read data successful!\n");

            for (int i = 0; i < 1024; i++)
            {
                printf("The data of address %d is %x \n", i, pData[i]);
            }
            break;

        case 'e':
            write_control(control_base, 0x0000, 0xFFFFFFFF);
            break;

        case 'r':
            write_control(control_base, 0x0004, 0xFFFFFFFF);
            break;
        }
    }
    work = 0;

    close(c2h_fd);
    close(h2c_fd);
    close(control_fd);
    close(interrupt_fd);

END:
    exit(0);
}


//根据CPU创建和分配线程
void ptd_create(pthread_t *arg, void *functionbody) {
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
	
	pthread_create(arg, &attr, (void *(*)(void *))functionbody, NULL);
    //printf("Id为%d的线程已创建完毕。", *arg);
	
	pthread_attr_destroy(&attr);//销除线程属性

    ptd_alarm = -1;
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
		printf("创建第%d个线程...", i);
        ptd_alarm = 1;
        ptd_create(&ptd[i], (void *)data_send);//这里根据需要更改
		while(1) {
			if(ptd_alarm < 0) {
				break;
			}
		}
	}

    for(int i = 0; i < CLIENT_NUM; i++){
        printf("等待客户端连接...");
        acfd[i] = accept(socket_fd, (struct sockaddr *)&clientaddr[i], &len);
        if(acfd[i] < 0){
            perror("Accept fail");
            exit(1);
        }
        printf("第%d个客户端已连接!", i);//希望能够显示连接的客户端地址
    }
}

void socket_send(){
    char channle_id_num[CLIENT_NUM][4] = {0};
    char frame_count[8] = {0};
    char id[20] = {0};


}

//数据分发
char pack_send[CLIENT_NUM][PACK_SIZE];//
int pack_count[CLIENT_NUM] = {0};//在data_send函数中需要在发送后将对应的count归零

void *data_part(){
    char channle_id[8] = {'\0'};
    int cpy_count = 0;
    int ret;
    char board_head[32];

    memset(&pack_send, '\0', sizeof(pack_send));
    while(count == 0){}
    while (part_alarm == -1){}

    memcpy(board_head, pData, 32);
    for (int i = 0; i < CLIENT_NUM; i++)
    {
        ret = send(acfd[i], board_head, 32, 0);
        if (ret < 0)
        {
            printf("%d: Send board_head info failed!", i);
            exit(1);
        }
    }
    printf("Board_head info send successful!");
    cpy_count++;

    for (; cpy_count * 32 < MAP_SIZE; cpy_count++){
        memcpy(channle_id, pData + cpy_count * 32, 32);
        ret = atoi(channle_id);
        if(ret < 20){
            pack_count[ret%CLIENT_NUM]++;
            memcpy(pack_send[ret%CLIENT_NUM] + pack_count[ret%CLIENT_NUM]*32, pData + cpy_count*32, 32);
        }else{
            printf("count: %d , cpy_count: %d :Data is discontinuous!", count, cpy_count);//这里需要做数据不连续的处理
        }
        cpy_count++;
    }
    cpy_count = 0;
    part_alarm = -1;
    //这里增加data_send发送函数

    while(1){
        while(part_alarm == -1){}

        part_alarm = -1;
    }




    return NULL;
}

//数据发送
void *data_send(){
    return NULL;
}

int main() {

}