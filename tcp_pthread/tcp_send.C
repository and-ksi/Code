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
#include <time.h>
#include <random>
#include <sys/time.h>
#include <pthread.h>

#define CPU_CORE (1)

typedef struct msg {
	char message[1024];
	int t, id, acfd;
	pthread_t ptd[CPU_CORE];
}MSG;

void ptd_create(void *arg);
void *msgsend(void *arg);

int main(int argc, char const *argv[]) {
	MSG msg;
	memset(&msg, 0, sizeof(msg));
	int fd, ret;
	struct sockaddr_in localaddr = {0};
	struct sockaddr_in clientaddr = {0};
	socklen_t len;

	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(10000);
	localaddr.sin_addr.s_addr = INADDR_ANY;

	FILE *fp = fopen("tcp_speeds.log", "w+");
	if(fp < 0){
		printf("File create fail!\n");
		exit(1);
	}

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0) {
		printf("Socket fail!\n");
		exit(1);
	}
	ret = bind(fd, (struct sockaddr *)&localaddr, sizeof(localaddr));
	if(ret < 0) {
		printf("Bind fail!\n");
		exit(1);
	}
	ret = listen(fd, 10);
	if(ret < 0) {
		printf("Listen fail!\n");
		exit(1);
	}

	for(int i = 0; i < CPU_CORE; i++) {
		printf("等待连接中......");
		msg.acfd = accept(fd, (struct sockaddr *)&clientaddr, &len);
		ptd_create((void *)&msg);
		while(1) {
			if(msg.id < 0) {
				break;
			}
		}
		msg.id = i;
		
	}



	return 0;
}

void *msgsend(void *arg) {
	return NULL;
}

void ptd_create(void *arg, void *functionbody) {
	int ret;

	cpu_set_t cpusetinfo;
	CPU_ZERO(&cpusetinfo);
	CPU_SET(1, &cpusetinfo);//将core1加入到cpu集中,同理可以将其他的cpu加入


	pthread_t ptd;
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
	
	pthread_create(&ptd, &attr, (void *(*)(void *))functionbody, (void *)arg);
	
	pthread_attr_destroy(&attr);//消除线程属性
}