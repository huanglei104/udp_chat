#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

int sockfd;
char *username;

int start_send_msg()
{
	int ret;
	char msg[1024];
	struct sockaddr_in bcaddr;

	memset(&bcaddr, 0, sizeof(bcaddr));
	bcaddr.sin_family = AF_INET;
	inet_aton("255.255.255.255", &bcaddr.sin_addr);
	bcaddr.sin_port = htons(4567);

	while (1) {
		memset(&msg, 0, 1024);
		sprintf(msg, "[%s]", username);

		scanf("%s", msg + strlen(username) + 2);

		ret = sendto(sockfd, msg, strlen(msg), 0,
				(struct sockaddr*)&bcaddr,
				sizeof(bcaddr));
		if (ret == -1) {
			perror("sendto");
			continue;
		}
	}

	close(sockfd);
}

int init(int argc, char *argv[])
{
	int ret;
	int optval;
	struct sockaddr_in local_addr;

	if (argc != 2) {
		printf("no username\n");
		return EXIT_FAILURE;
	}

	username = argv[1];

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket");
		return EXIT_FAILURE;
	}

	optval = 1;

	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
			&optval,
			sizeof(optval));
	if (ret == -1) {
		perror("setsockopt SO_REUSEADDR");
		close(sockfd);
		return EXIT_FAILURE;
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htons(INADDR_ANY);
	local_addr.sin_port = htons(4567);

	ret = bind(sockfd, (struct sockaddr*)&local_addr,
			sizeof(local_addr));
	if (ret == -1) {
		perror("bind");
		close(sockfd);
		return EXIT_FAILURE;
	}

	ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
			&optval,
			sizeof(optval));
	if (ret == -1) {
		perror("setsockopt SO_BROADCAST");
		close(sockfd);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void* recv_msg(void *args)
{
	char buf[1024];

	while (1) {
		memset(buf, 0, 1024);
		recvfrom(sockfd, buf, 1024, 0, NULL, NULL);
		printf("%s\n", buf);
	}

	return NULL;
}

void start_recv_msg()
{
	pthread_t recv_thread;

	pthread_create(&recv_thread, NULL, recv_msg, NULL);
}

int main(int argc, char *argv[])
{
	if (init(argc, argv) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	start_recv_msg();

	start_send_msg();

	return 0;
}
