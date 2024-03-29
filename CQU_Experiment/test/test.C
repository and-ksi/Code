/* #include <stdio.h>
#include <string.h>

typedef struct board_head
{
    char board_type[8];
    char board_addr[8];
    char Ftype[2];
    char Error[14];
} BOARD_HEAD;

int main(){
    BOARD_HEAD board_head;
    memset(&board_head, '0', sizeof(board_head));
    memset(&board_head.board_type, '1', 8);
    memset(&board_head.board_addr, '2', 8);
    memset(&board_head.Ftype, '3', 2);
    memset(&board_head.Error, '5', 14);

    char buf[10];
    memset(&buf, '1', 10);
    char ll[32];
    memset(&ll, '0', 32);
    memcpy(&ll, (const void *)&board_head, 32);
    int a = 39;
    sprintf(buf, "%d", a);
    printf("Struct body: %s\n", ll);
    printf("Buf : %s\n", buf);
    putchar(buf[3]);
    return 0;
} */
//  1.struct是按照字符顺序存储和读取的
//  2.向数组中注入数字会自动在后边加入结束字符

//测试信号量
/* #include <stdio.h>
#include <string.h>

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t semProducer;
sem_t semCustomer;

typedef struct node //节点结构
{
    int data;
    struct node *next;
} Node;

Node *g_head = nullptr; //永远指向链表头部的指针

void *funProducer(void *arg); //生产者--添加一个头结点
void *funCustomer(void *arg); //消费者--删除一个头结点

int main(int argc, char *argv[])
{
    pthread_t p1;
    pthread_t p2;

    sem_init(&semProducer, 0, 4); //初始化生产者线程信号量， （赋予 4 个，对比下一行，让生产者先运行）
    sem_init(&semCustomer, 0, 0); //初始化消费者线程信号量， (赋予 0 个， 一开始就让消费者处于阻塞状态)

    pthread_create(&p1, nullptr, funProducer, nullptr); //创建生产者线程
    pthread_create(&p2, nullptr, funCustomer, nullptr); //创建消费者线程

    pthread_join(p1, nullptr); //阻塞回收子线程
    pthread_join(p2, nullptr);

    sem_destroy(&semProducer); //销毁生产者信号量
    sem_destroy(&semCustomer); //销毁消费者信号量

    return 0;
}

void *funProducer(void *arg)
{
    while (true)
    {
        sem_wait(&semProducer); //semProducer--，  == 0， 则阻塞
        Node *pNew = new Node();
        pNew->data = rand() % 1000;
        pNew->next = g_head;
        g_head = pNew;
        printf("-----funProducer(生产者): %lu, %d\n", pthread_self(), pNew->data);
        sem_post(&semCustomer); // semCustomer++

        //sleep(rand() % 3); //随机休息 0~2 s
    }

    return nullptr;
}

void *funCustomer(void *arg)
{
    while (true)
    {
        sem_wait(&semCustomer); //semCustomer--，  == 0， 则阻塞
        Node *pDel = g_head;
        g_head = g_head->next;
        printf("-----funCustomer(消费者): %lu, %d\n", pthread_self(), pDel->data);
        delete (pDel);
        sleep(1);
        sem_post(&semProducer); // semProducer++
    }

    return nullptr;
} */

//十六进制数的读取和输出
/* #include <stdio.h>

int main(){
    unsigned int test[10] = {0};
    scanf("%x", test);
    printf("%d", *test);
    return 0;
} */

//通过管线控制gnuplot绘图
// #include <stdio.h>
// int main()
// {
//     FILE *gp;
// #ifdef WIN32 gp = _popen("gnuplot -persist", "w");
// #else gp = popen("gnuplot -persist", "w");
// #endif if (gp == NULL) return -1;
//     fprintf(gp, "set isosample 100/n");
//     fprintf(gp, "min=-1/n");
//     fprintf(gp, "max=1/n");
//     fprintf(gp, "pi=3.141592/n");
//     fprintf(gp, "set hidden3d/n");
//     fprintf(gp, "set pm3d/n");
//     fprintf(gp, "set contour/n");
//     fprintf(gp, "splot [min:max] [min:max] x*x+2*y*y-0.3*cos(3*pi*x)-0.4*cos(4*pi*y)+0.7/n");
//     fprintf(gp, "pause -1/n");
// #ifdef WIN32 _pclose(gp);
// #else pclose(gp);
// #endif return 0;
// }

#include <stdio.h>

int main(){
    FILE *fp = fopen("/home/cqu1/and/Code/data_analys/datalog/savelog_4-27_16-6-45.log", "r");
    if(fp == NULL){
        printf("read file failed!\n");
        perror("read");
        return 0;
    }
    unsigned int k[10000];
    fread(k, 4, 10000, fp);
    for(int i = 0; i < 1000; i++){
        printf("%x, \n", k[i]);
    }
    //lslslsls
    return 0;
}