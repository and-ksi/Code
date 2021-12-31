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
	char message[msgsize];
	int t, acfd;
}MSG;

void *msgsend(void *arg);
void dataproduct(void *arg);

int main(int argc, char const *argv[]) {
	int fd, ret, t1 = 0;
	long int starttime, time1, time2;
	int speed1, speed2;
	int on = 1;
	char buf[40] = {0};
	MSG msg;
	memset(&msg, 0, sizeof(msg));
	struct sockaddr_in localaddr = {0};
	struct sockaddr_in clientaddr = {0};
	socklen_t len;
	struct timeval time;

	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(10000);
	localaddr.sin_addr.s_addr = INADDR_ANY;

	dataproduct((void *)&msg.message);

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
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
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

	pthread_t ptd;
	msg.acfd = accept(fd, (struct sockaddr *)&clientaddr, &len);
	if(msg.acfd < 0) {
		printf("Accept fail!\n");
		exit(1);
	}
	pthread_create(&ptd, 0, msgsend, (void *)&msg);

	gettimeofday(&time, 0);
	starttime = time1 = time2 = time.tv_sec;
	while(time1 - starttime < 600) {
		gettimeofday(&time, 0);
		time1 = time.tv_sec;
		if(time1 - time2 >= 1) {
			speed1 = 8*(msg.t - t1)/1024/(time1 - time2);
			speed2 = 8*msg.t/1024/(time1 - starttime);
			printf("%lds 	已发送%dMB, 	当前传输速度%dMB/s, 	总传输速度%dMB/s.\n", 
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
	close(msg.acfd);
	close(fd);

	return 0;
}

void *msgsend(void *arg) {
	MSG *parg = (MSG *)arg;
	int ret;

	while(1) {
		ret = write(parg->acfd, parg->message, sizeof(parg->message));
		if(ret < 0) {
			printf("Send fail!\n");
			exit(1);
		}
		parg->t++;
	}
	return NULL;
}

void dataproduct(void *arg) {
	char *parg = (char *)arg;
	int i;
	srand(time(NULL));
	for(i = 0; i < sizeof(*parg) - 1; i++) {
		parg[i] = 'a' + rand()%26;
	}
	parg[i] = '\0';
}