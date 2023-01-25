#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 4000

int connected = 0;

void *my_thread(void *arg)
{
	int i;
	int fd = *(int *)arg;
	char s[256];
	int n;

	do
	{
		n = read(fd, s, 256);
		printf("%s\n", s);
	} while (n > 0);

	connected = 0;
	close(fd);

	return NULL;
}

int main(int argc, char **argv)
{
	struct sockaddr_in ad;
	int fd;
	pthread_t tid;
	int port = PORT;

	if (argc >= 2)
	{
		sscanf(argv[1], "%d", &port);
		printf("port = %d\n", port);
	}
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket error\n");
		return -1;
	}

	memset(&ad, 0, sizeof(ad));
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = inet_addr("127.0.0.1");
	ad.sin_port = htons(port);

	if (connect(fd, (struct socket_addr *)&ad, sizeof(ad)) != 0)
	{
		printf("connection failed\n");
		return -1;
	}

	connected = 1;
	pthread_create(&tid, NULL, my_thread, &fd);
	printf("connect on\n");
	while (connected)
	{
		char buf[256];
		scanf("%s", buf);
		write(fd, buf, strlen(buf) + 1);
		if (strcmp(buf, "quit") == 0)
		{
			sleep(1); 
			close(fd);
			exit(0);
		}
	}
}
