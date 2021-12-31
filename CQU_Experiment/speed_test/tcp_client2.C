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
	int fd, ret, t = 0, t1 = 0;
	char msg[MSG_SIZE] = {0}, buf[10] = {0};
	struct timeval time1, time2, starttime;
	float speed1, speed2;

	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10000);
	addr.sin_addr.s_addr = inet_addr(argv[1]);

	FILE *fp;
	fp = fopen("speedc.txt", "w+");
	if (fp == NULL) {
		printf("File fail!\n");
		exit(1);
	}

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket fail");
		exit(1);
	} else {
		printf("Socket success!\n");
	}

	ret = connect (fd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
	if (ret < 0 ) {
		perror("Connect fail");
		exit(1);
	} else {
		printf("Connect success!\n");
	}

	ret = write (fd, "OK", 2);
	if (ret < 0) {
		perror("OK fail");
		exit(1);
	} else {
		printf("OK send!\n");
	}

	gettimeofday(&starttime, 0);
	gettimeofday(&time1, 0);
	gettimeofday(&time2, 0);
//	time1 = time2 = starttime = clock()/CLOCKS_PER_SEC;
	while((memset(msg, 0, MSG_SIZE), read(fd, msg, MSG_SIZE)) > 0) {
		t++;
		gettimeofday(&time1, 0);
		if ((time1.tv_sec - time2.tv_sec) >= 1) {
			speed1 = 8*(t-t1)/1024/(time1.tv_sec - time2.tv_sec);
			speed2 = 8*t/1024/(time1.tv_sec - starttime.tv_sec);
			gettimeofday(&time2, 0);
			printf("%ld: 已传输%dMB, 当前速度%fMB/s, 全局速度%fMB/s.\n", time1.tv_sec, 8*t/1024, speed1, speed2);
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
	fclose(fp);

	return 0;
}