#include "ana.h"

//PCIE读取 变量
int c2h_fd[2];
int h2c_fd;
int control_fd;
void *control_base;
int interrupt_fd;
int read_end_addr;

void *data_read(){
    interrupt_fd = open_event("/dev/xdma0_events_0"); //打开用户中断
    while (work != -1)
    {
        sem_wait(sem + 0);
        printf("debug: Start read interrupt !\n");
        read_event(interrupt_fd); //获取用户中断
        printf("debug: read_event finished\n");
        read_end_addr = (int)read_control(control_base, 0x0008); //read interrupt reg
        write_control(control_base, 0x0000, 0xFFFFFFFF);         //ack interrupt

        printf("debug: Start read data !\n");
        //printf("read_end_addr = %x \n", read_end_addr);
        lseek(c2h_fd[id], (read_end_addr - (50 - id * 25) * 1024 * 1024), SEEK_SET);
        read(c2h_fd[id], pData, sizeof(pData)); //10MB

        printf("debug: read data finished!\n");
        sem_post(sem + 1);
    }
}