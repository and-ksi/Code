#include "recv_ana.h"

int work;

int c2h_fd;
int h2c_fd;
int control_fd;
void *control_base;
int interrupt_fd;

int read_end_addr;

int pData_0[RX_SIZE / 4];
int pData_1[RX_SIZE / 4];

int cnt;

static sem_t int_sem_rx;

//write bypass devies
static void write_control(void *base_addr, int offset, uint32_t val)
{
    //uint32_t writeval = htoll(val);
    *((uint32_t *)(base_addr + offset)) = val;
}

/*中断处理进程*/
void *event_process()
{
    interrupt_fd = open_event("/dev/xdma0_events_0"); //打开用户中断
    while (work == 1)
    {
        //printf("Start read interrupt !\n");
        read_event(interrupt_fd);                           //获取用户中断
        read_end_addr = read_control(control_base, 0x0008); //read interrupt reg
        fd = open(devicename, O_RDWR | O_SYNC);
        if (fd == -1)
        {
            printf("open event error\n");
            exit(1);
        }else
            printf("open event successful!\n");                 //ack interrupt
        sem_post(&int_sem_rx);                              //release signal
        //printf("read_end_addr = %x \n",read_end_addr);
        //lseek(c2h_fd, read_end_addr, SEEK_SET);
        //read(c2h_fd, pData_0, 32*1024);
        //cnt = cnt +1;
        //printf("cnt = %d\n",cnt);
        //printf("Stop read!\n");
    }
    pthread_exit(0);
}