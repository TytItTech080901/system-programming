#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#define BUF_SIZE 500
#define MAX_EVENTS 1024

static int set_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return -1;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(int argc, char *argv[])
{
	int sfd, ret; /* socket fd, return value */
	struct addrinfo hints; /* hints */
	struct addrinfo *result, *rp; /* for loop */
	int epfd, nfds; /* tcp listen, epoll fd, */
	struct epoll_event ev, events[MAX_EVENTS]; /* parameter for epoll */

	if (argc != 2) {
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	ret = getaddrinfo(NULL, argv[1], &hints, &result);
	if (ret != 0) {
		fprintf(stderr, "line %d :getaddrinfo: %s\n", __LINE__, gai_strerror(ret));
		exit(EXIT_FAILURE);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;
		int opt = 1;
		setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
			break;
		}

		close(sfd);
	}
	freeaddrinfo(result);

	if (rp == NULL) {
		fprintf(stderr, "line %d :Could not bind\n", __LINE__);
		exit(EXIT_FAILURE);
	}

	if (listen(sfd, SOMAXCONN) == -1) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	if (set_nonblock(sfd) == -1) {
		fprintf(stderr, "set_nonblock: error\n");
		exit(EXIT_FAILURE);
	}

	epfd = epoll_create1(0);
	if (epfd == -1) {
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = sfd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) == -1) {
		perror("epoll_ctl: sfd");
		exit(EXIT_FAILURE);
	}

	printf("Server listening on port %s (epoll)\n", argv[1]);

	for (;;) {
		nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			break;
		}

		for (int i = 0; i < nfds; i++) {
			int fd = events[i].data.fd;

			if (events[i].events & (EPOLLERR | EPOLLHUP)) {
				close(fd);
				epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
				printf("Connetion closed (error/hup)\n");
				continue;
			}

			if (fd == sfd) {
				for (;;) {
					struct sockaddr_storage cl_addr;
					socklen_t cl_len = sizeof(cl_addr);
					int cl_fd = accept(sfd, (struct sockaddr *)&cl_addr, &cl_len);
					if (cl_fd == -1) {
						if (errno == EAGAIN || errno == EWOULDBLOCK)
							break;
						perror("accept");
						break;
					}

					set_nonblock(cl_fd);
					ev.events = EPOLLIN;
					ev.data.fd = cl_fd;
					if (epoll_ctl(epfd, EPOLL_CTL_ADD, cl_fd, &ev) == -1) {
						perror("epoll_ctl: sfd");
						exit(EXIT_FAILURE);
					} else {
						printf("New client conneted: fd=%d\n", cl_fd);
					}
				}
			} else {
				char buffer[BUF_SIZE];
				int nread = recv(fd, buffer, sizeof(buffer) - 1, 0);

				if (nread <= 0) {
					if (nread == 0)
						printf("Client fd=%d closed\n", fd);
					else
						perror("recv");
					close(fd);
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
				} else {
					buffer[nread] = '\0';
					printf("Recieved from fd=%d: %s", fd, buffer);
					send(fd, buffer, nread, 0);
				}
			}
		}
	}

	close(sfd);
	close(epfd);
	return 0;
}
