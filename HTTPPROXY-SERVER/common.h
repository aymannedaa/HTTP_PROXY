#ifndef COMMON_H
#define COMMON_H

#include <sys/socket.h>

#define LISTENQ 5
#define	MAXN	100000		/* max # bytes to request from server */
#define MAXLINE 1024

int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp);
void web_child(long long sockfd);
void pr_cpu_time(void);
void sig_chld(int signo);
int tcp_connect(const char *host, const char *serv);
ssize_t readn(int fd, void *vptr, size_t n);

ssize_t read_fd(int fd, void *ptr, size_t nbytes, int *recvfd);
ssize_t write_fd(int fd, void *ptr, size_t nbytes, int sendfd);

#endif
