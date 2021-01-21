#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

int recvfd, sendfd;
char *username;

void* recv_process(void *args)
{
	int optval, ret;
	char msg[1024];
	socklen_t len;
	struct sockaddr_in recvAddr;

	memset(msg, 0, 1024);

	recvfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (recvfd == -1) {
		perror("socket");
		return NULL;
	}

	optval = 1;
	len = sizeof(recvAddr);

	ret = setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
	if (ret == -1) {
		perror("setsockopt");
		close(recvfd);
		return NULL;
	}

	memset(&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_port = htons(4001);
	recvAddr.sin_addr.s_addr = INADDR_ANY;

	// 必须绑定，否则无法监听
	ret = bind(recvfd, (struct sockaddr*)&recvAddr, len);
	if (ret == -1) {
		perror("bind");
		close(recvfd);
		return NULL;
	}

	while (1) {
		//ret = recvfrom(recvfd, msg, 1024, 0, (struct sockaddr*)&recvAddr, &len);
		ret = recv(recvfd, msg, 1024, 0);
		if (ret == -1) {
			perror("recvfrom");
			continue;
		}

		printf("%s\n", msg);
	}

	close(recvfd);

	return NULL;
}

int send_msg()
{
	int ret;
	struct sockaddr_in bcaddr;
	char msg[1024];

	memset(&bcaddr, 0, sizeof(struct sockaddr_in));
	memset(&msg, 0, 1024);
	bcaddr.sin_family = AF_INET;
	inet_aton("255.255.255.255", &bcaddr.sin_addr);
	bcaddr.sin_port = htons(4001);

	sprintf(msg, "[%s]", username);

	while (1) {
		scanf("%s", msg + strlen(username) + 2);

		ret = sendto(sendfd, msg, strlen(msg), 0, (struct sockaddr*)&bcaddr, sizeof(bcaddr));
		if (ret == -1) {
			perror("sendto");
			continue;
		}
	}

	close(sendfd);
}

int main(int argc, char *argv[])
{
	int ret;
	int optval;
	pthread_t recv_thread;

	if (argc != 2) {
		printf("error: no username\n");
		return 0;
	}

	optval = 1;
	username = argv[1];

	sendfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sendfd == -1) {
		perror("socket");
		return 0;
	}

	ret = setsockopt(sendfd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, &optval, sizeof(int));
	if (ret == -1) {
		perror("setsockopt");
		close(sendfd);
		return 0;
	}

	pthread_create(&recv_thread, NULL, recv_process, NULL);

	send_msg();

	return 0;
}
