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

#define MSG_SIZE (1024)

typedef struct msg {
	//char message[MSG_SIZE];
	int t, fd;
}MSG;

void dataproduct(void *arg);
void *msgsend(void *arg);

int main(int argc, char const *argv[]) {
	MSG msg = {0};
	int ret, on = 1, t1 = 0;
	char buf[40];
	struct timeval time;
	long int starttime, time1, time2;
	int speed1, speed2;

	FILE *fp = fopen("udp_speeds.log", "w+");
	if(fp < 0){
		printf("File create fail!\n");
		exit(1);
	}

	msg.fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(msg.fd < 0) {
		printf("Socket fail!\n");
		exit(1);
	}
	setsockopt(msg.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	pthread_t ptd;
	pthread_create(&ptd, 0, msgsend, (void *)&msg);
	gettimeofday(&time, 0);
	starttime = time1 = time2 = time.tv_sec;
	while(time1 - starttime < 600) {
		gettimeofday(&time, 0);
		time1 = time.tv_sec;
		if(time1 - time2 >= 1) {
			speed1 = (msg.t - t1)/1024/(time1 - time2);
			speed2 = msg.t/1024/(time1 - starttime);
			printf("%lds 	已发送%dMB, 	当前传输速度%dMB/s, 	总传输速度%dMB/s.\n", 
				time1 - starttime, msg.t/1024, speed1, speed2);
			sprintf(buf, "%ld s 	%d MB 	%d MB/s		%d MB/s 	\n",
			 time1 - starttime, msg.t/1024, speed1, speed2);
			fputs(buf, fp);
			time2 = time1;
			t1 = msg.t;
		}
	}
	fclose(fp);
	close(msg.fd);

	return 0;
}

void *msgsend(void *arg) {
	MSG *parg = (MSG *)arg;
	char message[MSG_SIZE] = {0};
	int ret;

	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10000);
	inet_pton(AF_INET, "192.168.3.2", &addr.sin_addr.s_addr);

	dataproduct((void *)&message);

	while(1) {
		ret = sendto(parg->fd, message, sizeof(message), 0, (sockaddr *)&addr, sizeof(addr));
		if(ret < 0) {
			printf("Send fail!\n");
			break;
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