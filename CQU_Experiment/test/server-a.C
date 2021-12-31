#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
 
#define BUF_SIZE  (8192)

unsigned char fileBuf[BUF_SIZE];
 
/*
 * send file
 */
void
//file_server(const char *path)
file_server()
{
	
    int skfd, cnfd;
//    FILE *fp = NULL;
    struct sockaddr_in sockAddr, cltAddr;//这三个定义在哪？
    socklen_t addrLen;//这五个参数是关于socket的函数名，具体用法即struct ...
//    unsigned int fileSize;
    int netSize;
//    char buf[10];
 
/*    if( !path ) {
        printf("file server: file path error!\n");
        return;
    }*/
 
    //创建tcp socket
    if((skfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");//如果报错，将会是socket:.......
        exit(1);//需要注意的是,socket调用成功返回0,如调用失败返回-1
    } else {
        printf("socket success!\n");
    }
 
    //创建结构  绑定地址端口号
    memset(&sockAddr, 0, sizeof(struct sockaddr_in));//将ip地址(sockaddr_in)长度的0填入sockaddr后的位数中
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockAddr.sin_port = htons(10002);//htons转换网络字节和本地字节顺序
 
    //bind绑定端口?
    if(bind(skfd, (struct sockaddr *)(&sockAddr), sizeof(struct sockaddr)) < 0) {
        perror("Bind");
        exit(1);
    } else {
        printf("bind success!\n");
    }
 
    //listen   监听  最大4个用户
    if(listen(skfd, 4) < 0) {
        perror("Listen");
        exit(1);
    } else {
        printf("listen success!\n");
    }
 
    /* 调用accept,服务器端一直阻塞，直到客户程序与其建立连接成功为止*/
    addrLen = sizeof(struct sockaddr_in);//初始化addrlen
    if((cnfd = accept(skfd, (struct sockaddr *)(&cltAddr), &addrLen)) < 0) {
        perror("Accept");
        exit(1);
    } else {
        printf("accept success!\n");
    }
 
    /*fp = fopen(path, "r");
    if( fp == NULL ) {
        perror("fopen");
        close(cnfd);
        close(skfd);
        return;
    }*/
 
//    fseek(fp, 0, SEEK_END);//用来移动指针到结尾
//    fileSize = ftell(fp);//将文件结尾的位置赋值
//    fseek(fp, 0, SEEK_SET);//指针移动到开头
 
/*    if(write(cnfd, (unsigned char *)&fileSize, 4) != 4) {
        perror("write");//cnfd是连接套接字
        close(cnfd);
        close(skfd);
        exit(1);
    }
 
    if( read(cnfd, buf, 2) != 2) {
        perror("read");
        close(cnfd);
        close(skfd);
        exit(1);
    }
*/
    int a[BUF_SIZE];
	for(int i=0;i<(BUF_SIZE*10);i++){
		a[i]=rand()%10;
	}
	unsigned int size;
	size = BUF_SIZE*10;

//    while(size2<(BUF_SIZE*10)) {
        unsigned int size2 = 0;
        while( size2 < size) {
            if( (netSize = write(cnfd, a, size - size2)) < 0 ) {
                perror("write");
                close(cnfd);
                close(skfd);
                exit(1);
            }
            size2 += netSize;
        }
//    }
 
//    fclose(fp);
    close(cnfd);
    close(skfd);
}
 
//int main(int argc, char **argv)
int main()
{
    /*if( argc < 2 ) {
        printf("file server: argument error!\n");
        printf("file_server /tmp/temp\n");
        return -1;
    }*/
 
    file_server();
 
    return 0;
}

//创建随机数据
/*	int a[BUF_SIZE];
	for(int i=0;i<BUF_SIZE;i++){
		a[i]=ran_num=rand()%10;
	}*/