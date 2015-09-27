// References:
// Server example 6 (concurrent server, one thread per client)
// of lecture 6 , modified from W.R. Stevens UNPv1e3 example, Section 30.10
// The Deamon function here is copied 'as is' from Deamon example given in lecture

// To build:
// gcc -o HTTPServer HTTPServer.c common.c getput.c -lpthread
// (i.e., common.c has the common functions used in this file,
// and getput has the GET and PUT requests/response functions)

#include "common.h" // From Network programming lecture 6 examples for support functions in common.c
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <syslog.h>

#define	MAXFD	64


// parameters: pname process name
// facility: syslog facility
int daemon_init(const char *pname, int facility)
{
	int	i;
	pid_t	pid;

	/* Create child, terminate parent
	   - shell thinks command has finished, child continues in background
	   - inherits process group ID => not process group leader
	   - => enables setsid() below
	 */
	if ( (pid = fork()) < 0)
		return -1;  // error on fork
	else if (pid)
		exit(0);			/* parent terminates */

	/* child 1 continues... */

	/* Create new session
	   - process becomes session leader, process group leader of new group
	   - detaches from controlling terminal (=> no SIGHUP when terminal
	     session closes)
	 */
	if (setsid() < 0)			/* become session leader */
		return -1;

	/* Ignore SIGHUP. When session leader terminates, children will
	   will get SIGHUP (see below)
	 */
	signal(SIGHUP, SIG_IGN);

	/* Create a second-level child, terminate first child
	   - second child is no more session leader. If daemon would open
	     a terminal session, it may become controlling terminal for
	     session leader. Want to avoid that.
	 */
	if ( (pid = fork()) < 0)
		return -1;
	else if (pid)
		exit(0);			/* child 1 terminates */

	/* child 2 continues... */

	/* change to "safe" working directory. If daemon uses a mounted
	   device as WD, it cannot be unmounted.
	 */
	chdir("/");				/* change working directory */

	/* close off file descriptors (including stdin, stdout, stderr) */
	// (may have been inherited from parent process)
	for (i = 0; i < MAXFD; i++)
		close(i);

	/* redirect stdin, stdout, and stderr to /dev/null */
	// Now read always returns 0, written buffers are ignored
	// (some third party libraries may try to use these)
	// alternatively, stderr could go to your log file
	open("/dev/null", O_RDONLY); // fd 0 == stdin
	open("/dev/null", O_RDWR); // fd 1 == stdout
	open("/dev/null", O_RDWR); // fd 2 == stderr

	// open syslog
	openlog(pname, LOG_PID, facility);

	return 0;				/* success */
}


int main(int argc, char **argv)
{
	int		listenfd;
	long long	connfd;  // hack: to make this 64 bits for type cast
	void		sig_int(int);
	void*		doit(void *);
	pthread_t	tid;
	socklen_t	clilen, addrlen;
	struct sockaddr	*cliaddr;

// TODO: commandline option to set Daemon ON/OFF


	//daemon_init("HTTPServer", LOG_WARNING); // initialize Daemon process

 if (argc == 3)  // if only port specified
 {
	 if(strcmp(argv[2],"1") == 0 ){
		 printf("Server Started in the Background as Daemon\n");
		 daemon_init("HTTPServer", LOG_WARNING);
	 }else
		 printf("Server Started not as Daemon\n");


	 listenfd = tcp_listen(NULL, argv[1], &addrlen);
 }
	 else if (argc == 4) // if host and port specified
	 {
		 if(strcmp(argv[3],"1") == 0 ){
		 printf("Server Started in the Background as Daemon\n");
				 daemon_init("HTTPServer", LOG_WARNING);
			 }else
				 printf("Server Started not as Daemon\n");

		 listenfd = tcp_listen(argv[1], argv[2], &addrlen);
	 }
		 else {
		printf("usage: ./HTTPServer [ <host> ] <port#> <1: Daemon-ON otherwise Daemon-OFF>\n");
		return -1;
	}
	if (listenfd < 0) {
	  perror("tcp_listen");
	  exit(1);
	}

	cliaddr = malloc(addrlen);

	signal(SIGINT, sig_int);

	// accept client connections through listening socket
	for ( ; ; ) {
		clilen = addrlen;
		connfd = accept(listenfd, cliaddr, &clilen);
		if (connfd < 0) {
		  perror("accept");
		  exit(1);
		}

		if (pthread_create(&tid, NULL, &doit, (void *) connfd) != 0) {
			perror("pthread_create");
			return -1;
		}
	}
	closelog();
	return 0;
}


void *doit(void *arg)
{
	// ugly hack to cast *arg pointer into descriptor
	long long connfd = (long long) arg;

	pthread_detach(pthread_self());
	web_child(connfd); // handling each client
	close(connfd);

	return(NULL);
}


void sig_int(int signo)
{
	signo = signo;  // silence compiler warning

	void pr_cpu_time(void);

	pr_cpu_time();
	exit(0);
}
