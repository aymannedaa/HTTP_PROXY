// A modified version of common.c file used in lecture 6,
// supporting GET and PUT requests/responses instead of
// sending/receiving random bytes

#include "common.h"
#include "getputpost.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <syslog.h>

#define MAXBUF 20000


#define	HAVE_MSGHDR_MSG_CONTROL

// Establish a listening socket
int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
        int                     listenfd, n;
        const int               on = 1;
        struct addrinfo hints, *res, *ressave;


        bzero(&hints, sizeof(struct addrinfo));
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) {
                fprintf(stderr, "tcp_listen error for %s, %s: %s",
                                 host, serv, gai_strerror(n));
		return -1;
	}
        ressave = res;

        do {
                listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
                if (listenfd < 0)
                        continue;               /* error, try next one */

                setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
                if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
                        break;                  /* success */

                close(listenfd);        /* bind error, close and try next one */
        } while ( (res = res->ai_next) != NULL);

        if (res == NULL) {        /* errno from final socket() or bind() */
                fprintf(stderr, "tcp_listen error for %s, %s", host, serv);
		return -1;
	}

        if (listen(listenfd, LISTENQ) < 0) {
		perror("listen");
		return -1;
	}

        if (addrlenp)
                *addrlenp = res->ai_addrlen;    /* return size of protocol address */

        freeaddrinfo(ressave);

        return(listenfd);
}




void web_child(long long sockfd) // Handling each client by calling GET or PUT functions
{
	int buf_read, total;
	char buf[MAXBUF];

	syslog(LOG_INFO, "Handling client");
	// start reading from the socket, as long as connection is open
	//while ( (buf_read = read(sockfd, buf, MAXBUF)) > 0) {

	if (read(sockfd, buf, MAXBUF) > 0){

		syslog(LOG_INFO,"Received message:");
		syslog(LOG_INFO,buf);

        if ( strncmp(buf,"GET ", 4) == 0 )
        	get_request(buf,sockfd);

        else if (strncmp(buf,"PUT ", 4) == 0 )
        	put_request(buf,sockfd);

        else if (strncmp(buf,"POST ", 5) == 0 )
        	post_request(buf,sockfd);

        else
        {
        	write(sockfd,"HTTP/1.1 400 Bad Request\n", 25);
        }

        }
		close(sockfd);

	if (buf_read < 0) {
		perror("read error");
	}

}


void pr_cpu_time(void)
{
        double                  user, sys;
        struct rusage   myusage, childusage;

        if (getrusage(RUSAGE_SELF, &myusage) < 0) {
                perror("getrusage error");
		return;
	}
        if (getrusage(RUSAGE_CHILDREN, &childusage) < 0) {
                perror("getrusage error");
		return;
	}

        user = (double) myusage.ru_utime.tv_sec +
                                        myusage.ru_utime.tv_usec/1000000.0;
        user += (double) childusage.ru_utime.tv_sec +
                                         childusage.ru_utime.tv_usec/1000000.0;
        sys = (double) myusage.ru_stime.tv_sec +
                                   myusage.ru_stime.tv_usec/1000000.0;
        sys += (double) childusage.ru_stime.tv_sec +
                                        childusage.ru_stime.tv_usec/1000000.0;

        printf("\nuser time = %g, sys time = %g\n", user, sys);
}


// Process SIGCHLD: when child dies, must use waitpid to clean it up
// otherwise it will remain as zombie
void sig_chld(int signo)
{
        pid_t   pid;
        int             stat;

	//signo=signo; // silence warning

        while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
                /* printf("child %d terminated\n", pid); */
        }
        return;
}


int tcp_connect(const char *host, const char *serv)
{
        int                             sockfd, n;
        struct addrinfo hints, *res, *ressave;

        bzero(&hints, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) {
                fprintf(stderr, "tcp_connect error for %s, %s: %s",
                                 host, serv, gai_strerror(n));
		return -1;
	}

        ressave = res;

        do {
                sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
                if (sockfd < 0)
                        continue;       /* ignore this one */

                if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
                        break;          /* success */

                close(sockfd);  /* ignore this one */
        } while ( (res = res->ai_next) != NULL);

        if (res == NULL) {        /* errno set from final connect() */
		fprintf(stderr, "tcp_connect error for %s, %s", host, serv);
		return -1;
	}

        freeaddrinfo(ressave);

        return(sockfd);
}



