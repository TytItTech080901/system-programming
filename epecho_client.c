#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <errno.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(argv[1], argv[2], &hints, &res) != 0) {
		perror("getaddrinfo");
		exit(EXIT_FAILURE);
	}

	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
		perror("connect");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(res);

	printf("Connected to %s:%s\n", argv[1], argv[2]);

	int epfd = epoll_create1(0);
	struct epoll_event ev, events[2];
	ev.events = EPOLLIN;
	ev.data.fd = STDIN_FILENO; /* 把标准输入也加进去 */
	epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
	ev.data.fd = sockfd;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

	char line[BUF_SIZE], buf[BUF_SIZE];
	int running = 1;

	while (running) {
		int n = epoll_wait(epfd, events, 2, -1);
		for (int i = 0; i < n; i++) {
			int fd = events[i].data.fd;

			if (fd == STDIN_FILENO) {
				if (fgets(line, sizeof(line), stdin) == NULL) {
					running = 0;
					break;
				}
				send(sockfd, line, strlen(line), 0);
			} else if (fd == sockfd) {
				/* 服务端关闭时，内核会完成四次挥手，服务端会给客户端发送FIN包，让recv返回0，从而退出 */
				int r = recv(sockfd, buf, sizeof(buf) - 1, 0); 
				if (r <= 0) {
					printf("Server disconnected\n");
					running = 0;
					break;
				}
				buf[r] = '\0';
				printf("%s", buf);
			}
		}
	}

	close(sockfd);
	close(epfd);
	return 0;
}
