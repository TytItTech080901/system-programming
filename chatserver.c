#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "logger.h"

#define BUF_SIZE 500
#define MAX_EVENTS 1024
#define MAX_CLIENT 1024

static int client_fds[MAX_CLIENT];
static char client_names[MAX_CLIENT][INET6_ADDRSTRLEN];
static int client_now = 0;

static int set_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return -1;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static void add_client(int fd, const char *name)
{
	if (client_now < MAX_CLIENT) {
		client_fds[client_now] = fd;
		if (name)
			strncpy(client_names[client_now], name, sizeof(client_names[0]) - 1);
		client_names[client_now][sizeof(client_names[0]) - 1] = '\0';
		client_now++;
		LOG_INFO("CHAT", "Client %s (fd=%d) joined, total: %d", client_names[client_now - 1], fd,
				 client_now);
	} else {
		LOG_WARN("CHAT", "Max clients reached, rejected fd=%d", fd);
		close(fd);
	}
}

static void remove_client(int fd)
{
	for (int i = 0; i < client_now; i++) {
		if (client_fds[i] == fd) {
			LOG_INFO("CHAT", "Client %s (fd=%d) left, remaining: %d", client_names[i], fd,
					 client_now - 1);
			client_fds[i] = client_fds[client_now - 1];
			strncpy(client_names[i], client_names[client_now - 1], sizeof(client_names[0]) - 1);
			client_now--;
			break;
		}
	}
}

static const char *client_name_of(int fd)
{
	for (int i = 0; i < client_now; i++) {
		if (client_fds[i] == fd)
			return client_names[i];
	}
	return "?";
}

static void get_client_ip(int fd, struct sockaddr_storage *addr, char *buf, size_t bufsize)
{
	if (addr->ss_family == AF_INET) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)addr;
		inet_ntop(AF_INET, &ipv4->sin_addr, buf, bufsize);
	} else if (addr->ss_family == AF_INET6) {
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)addr;
		inet_ntop(AF_INET6, &ipv6->sin6_addr, buf, bufsize);
	} else {
		snprintf(buf, bufsize, "unknown");
	}
}

static void broadcast(int sender_fd, const char *msg, int msg_len)
{
	for (int i = 0; i < client_now; i++) {
		int fd = client_fds[i];
		if (fd != sender_fd) {
			send(fd, msg, msg_len, MSG_NOSIGNAL);
		}
	}
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

	log_init(NULL, LOG_EVENT_DEBUG, 1000);
	log_add_output(stderr);

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
		LOG_ERROR("CHAT", "getaddrinfo: %s", gai_strerror(ret));
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
		LOG_ERROR("CHAT", "Could not bind to port %s", argv[1]);
		exit(EXIT_FAILURE);
	}

	if (listen(sfd, SOMAXCONN) == -1) {
		LOG_ERROR("CHAT", "listen: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (set_nonblock(sfd) == -1) {
		LOG_ERROR("CHAT", "set_nonblock failed");
		exit(EXIT_FAILURE);
	}

	epfd = epoll_create1(0);
	if (epfd == -1) {
		LOG_ERROR("CHAT", "epoll_create: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	ev.events = EPOLLIN;
	ev.data.fd = sfd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) == -1) {
		LOG_ERROR("CHAT", "epoll_ctl(sfd): %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	LOG_INFO("CHAT", "Server listening on port %s (epoll)", argv[1]);

	for (;;) {
		nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			LOG_ERROR("CHAT", "epoll_wait: %s", strerror(errno));
			break;
		}

		for (int i = 0; i < nfds; i++) {
			int fd = events[i].data.fd;

			if (events[i].events & (EPOLLERR | EPOLLHUP)) {
				close(fd);
				epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
				LOG_WARN("CHAT", "Connection fd=%d closed (error/hup)", fd);
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
						LOG_ERROR("CHAT", "accept: %s", strerror(errno));
						break;
					}

					set_nonblock(cl_fd);
					ev.events = EPOLLIN;
					ev.data.fd = cl_fd;
					epoll_ctl(epfd, EPOLL_CTL_ADD, cl_fd, &ev);

					// 获取客户端 IP
					char ip_str[INET6_ADDRSTRLEN];
					get_client_ip(cl_fd, &cl_addr, ip_str, sizeof(ip_str));

					// ★ 加入客户端列表
					add_client(cl_fd, ip_str);

					// ★ 广播：XX 进入了聊天室
					char join_msg[BUF_SIZE];
					snprintf(join_msg, sizeof(join_msg),
							 "*** [%s] joined the chatroom (total: %d) ***\n", ip_str, client_now);
					broadcast(cl_fd, join_msg, strlen(join_msg));
				}
			} else {
				char buffer[BUF_SIZE];
				int nread = recv(fd, buffer, sizeof(buffer) - 1, 0);

				if (nread <= 0) {
					// 连接断开
					close(fd);
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
					remove_client(fd);

					// ★ 广播：XX 离开了聊天室
					char leave_msg[BUF_SIZE];
					snprintf(leave_msg, sizeof(leave_msg),
							 "*** [%s] left the chatroom (remaining: %d) ***\n", client_name_of(fd),
							 client_now);
					broadcast(-1, leave_msg, strlen(leave_msg));
				} else {
					buffer[nread] = '\0';
					// 在服务器控制台打印
					LOG_INFO("CHAT", "[%s] %s", client_name_of(fd), buffer);

					// ★ 广播消息给所有其他客户端
					char chat_msg[BUF_SIZE + 64];
					snprintf(chat_msg, sizeof(chat_msg), "[%s] %s", client_name_of(fd), buffer);
					broadcast(fd, chat_msg, strlen(chat_msg));
				}
			}
		}
	}

	close(sfd);
	close(epfd);
	logger_shutdown();
	return 0;
}
