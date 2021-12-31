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

#define msgsize (8192)

typedef struct msg {
	//char message[1024];
	int t, fd;
}MSG;

void *msgrecv(void *arg);

int main(int argc, char const *argv[]) {
	MSG msg;
	int ret, t1 = 0;
	int speed1, speed2;
	char buf[40] = {0};
	struct timeval time;
	long int starttime, time1, time2;
	memset(&msg, 0, sizeof(msg));
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10000);
	inet_pton(AF_INET, "192.168.3.1", &addr.sin_addr.s_addr);

	FILE *fp = fopen("tcp_speedc.log", "w+");
	if(fp < 0){
		printf("File create fail!\n");
		exit(1);
	}

	msg.fd = socket(AF_INET, SOCK_STREAM, 0);
	if(msg.fd < 0) {
		printf("Socket fail!\n");
		exit(1);
	}
	ret = connect(msg.fd, (struct sockaddr *)&addr, sizeof(addr));
	if(ret < 0) {
		printf("Connect fail!\n");
		exit(1);
	}

	pthread_t ptd;
	pthread_create(&ptd, 0, msgrecv, (void *)&msg);
	
	gettimeofday(&time, 0);
	starttime = time1 = time2 = time.tv_sec;
	while(time1 - starttime < 600) {
		gettimeofday(&time, 0);
		time1 = time.tv_sec;
		if(time1 - time2 >= 1) {
			speed1 = 8*(msg.t - t1)/1024/(time1 - time2);
			speed2 = 8*msg.t/1024/(time1 - starttime);
			printf("%lds 	已接收%dMB, 	当前传输速度%dMB/s, 	总传输速度%dMB/s.\n", 
				time1 - starttime, 8*msg.t/1024, speed1, speed2);
			sprintf(buf, "%ld s 	%d MB 	%d MB/s		%d MB/s 	\n",
			 time1 - starttime, 8*msg.t/1024, speed1, speed2);
			fputs(buf, fp);
			time2 = time1;
			t1 = msg.t;
		}
	}
	pthread_cancel(ptd);
	fclose(fp);
	close(msg.fd);
	close(msg.fd);
	return 0;
}

void *msgrecv(void *arg) {
	MSG *parg = (MSG *)arg;
	char message[msgsize] = {0};
	int ret;

	while(1) {
		ret = read(parg->fd, message, sizeof(message));
		if(ret < 0) {
			printf("Recv fail!\n");
			break;
		}
		if(ret = 0) {
			printf("发送端已关闭!\n");
			break;
		}
		memset(&message, '\0', sizeof(message));
		parg->t++;
	}
	return NULL;
}