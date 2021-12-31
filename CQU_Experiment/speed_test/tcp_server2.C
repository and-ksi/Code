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

#define MSG_SIZE (8192)//8kB

int main(int argc, char const *argv[])
{
	int fd, acfd, t=0, t1=0, ret, i;
	char msg[MSG_SIZE] = {0}, buf[10] = {0};
	struct timeval starttime, time1, time2;
	float speed1, speed2;

	socklen_t addrLen;
	struct sockaddr_in addr = {0};
	struct sockaddr_in caddr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10000);
	addr.sin_addr.s_addr = INADDR_ANY;

	FILE *fp;
	fp = fopen("speeds.txt", "w+");
	if (fp == NULL) {
		printf("File fail!\n");
		exit(1);
	}

	srand(time(NULL));
	for (i = 0; i < MSG_SIZE; i++) {
		msg[i] = (char)rand()%100;
	}

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket fail");
		exit(1);
	} else {
		printf("Socket success!\n");
	}

	ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		perror("Bind fail");
		exit(1);
	} else {
		printf("Bind success!\n");
	}

	ret = listen(fd, 4);
	if (ret < 0) {
		perror("Listen fail");
		exit(1);
	} else {
		printf("Listen success!\n");
	}

	acfd = accept(fd, (struct sockaddr *)&caddr, &addrLen);
	if (acfd < 0) {
		perror("Accept fail");
		exit(1);
	} else {
		printf("Accept success!\n");
	}

	ret = read(acfd, buf, 2);
	if (ret != 2) {
		perror("OK fail");
		exit(1);
	} else {
		printf("OK success!\n");
		memset(buf, 0, 10);
	}

	gettimeofday(&starttime, 0);
	gettimeofday(&time1, 0);
	gettimeofday(&time2, 0);
//	starttime = time1 = time2 = clock()/CLOCKS_PER_SEC;
	while ((time1.tv_sec - starttime.tv_sec) < 600) {
		ret = write(acfd, msg, MSG_SIZE);
		if (ret < 0) {
			perror("Write fail");
			exit(1);
		}
		t++;

		gettimeofday(&time1, 0);
//		time1 = clock()/CLOCKS_PER_SEC;
		if ((time1.tv_sec - time2.tv_sec) >= 1) {
			speed1 = 8*(t - t1)/1024/(time1.tv_sec - time2.tv_sec);
			speed2 = 8*t/1024/(time1.tv_sec - starttime.tv_sec);
			printf("%lds: 已传输%dMB, 当前速度%fMB/s, 全局速度%fMB/s.\n", time1.tv_sec, 8*t/1024, speed1, speed2);
			gettimeofday(&time2, 0);;
			t1 = t;
			sprintf(buf, "%ld", time1.tv_sec);
			fputs(buf, fp);
			fputs("s 	", fp);
			sprintf(buf, "%d", 8*t/1024);
			fputs(buf, fp);
			fputs("MB 	", fp);
			sprintf(buf, "%f", speed1);
			fputs(buf, fp);
			fputs("MB/s 	" 	, fp);
			sprintf(buf, "%f", speed2);
			fputs(buf, fp);
			fputs("MB/s\n", fp);
		}
	}

	close(fd);
	close(acfd);
	fclose(fp);
	return 0;
}