#include     "unp.h"
#include     "unpthread.h"
#include     <time.h>

//deal with the SIG_PIPE signal-------------------------------------------------------------
void sig_pipe(int signo){
	printf("***Client termination: EPIPE error detected***\n");
	return;
}

//time thread method-------------------------------------------------------------------------
static void time_s(int sockfd){
	int 		maxfdp1, sfd;
	time_t		ticks;
	ssize_t		nread;
	char 		buff[MAXLINE], recvline[MAXLINE];
	fd_set		rset;
	struct timeval 	timeout;
	
	FD_ZERO(&rset);

	//initial report the time
	ticks = time (NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks) ) ;
        if(write(sockfd, buff, strlen(buff)) < 0)
		perror("time_s: write fails...");
	//loop to report time every 5 sec
	while(1){
		FD_SET(sockfd, &rset);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		if((sfd = select(sockfd+1, &rset, NULL, NULL, &timeout)) < 0){
			perror("time_s: select error...");
			break;
			}

		if(sfd == 0){
			ticks = time (NULL);
        		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks) ) ;
        		if(write(sockfd, buff, strlen(buff)) < 0){
				perror("time_s: write error...");
				break;
			}		
		}

		if(FD_ISSET(sockfd, &rset)){
			if((nread = read(sockfd, recvline, MAXLINE)) == 0){
				printf("***Client termination: socket read returned with value 0***\n");
				printf("***[DAYTIME] service connection terminates***\n");
				break;
			}
			else if(nread < 0 && errno == EINTR)
				continue;
			else if(nread < 0)
				err_sys("time_s: read error");
		}
	}
}
//-------------------------------------------------------------------------------------------
//time thread pointer------------------------------------------------------------------------
static void *time_serv(void *arg){
	int	connfd;	
	
	signal(SIGPIPE, sig_pipe);
	connfd = *((int *)arg);
	free(arg);
	
	pthread_detach(pthread_self());

 	time_s(connfd);

        close (connfd);
	return(NULL);
}

//echo thread method-------------------------------------------------------------------------
static void echo_s(int sockfd){
	ssize_t		nread;
	char 		recvline[MAXLINE];
	
	//loop to echo
	while(1){
		if((nread = readline(sockfd, recvline, MAXLINE)) == 0){
			printf("***Client termination: socket read returned with value 0***\n");
			printf("***[ECHO] service connection terminates***\n");
			break;
		}
		else if(nread > 0)
			if(writen(sockfd, recvline, nread) <= 0)
				perror("echo_s: write failed");

		else if(nread < 0 && errno == EINTR){
			perror("Client termination: socket read returned with value -1, errno = EINTR");
			continue;
		}
		else if(nread < 0)
			err_sys("echo_s: readline error");
	}
}
//-------------------------------------------------------------------------------------------
//echo thread pointer------------------------------------------------------------------------
static void *echo_serv(void *arg){
	int	connfd;	

	signal(SIGPIPE, sig_pipe);
	connfd = *((int *)arg);
	free(arg);
	
	pthread_detach(pthread_self());

 	echo_s(connfd);

        close (connfd);
	return(NULL);
}
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int     	listenecho, listentime, *iptr_time, *iptr_echo, maxfdp1;
	pthread_t 	tid;
        socklen_t 	addrlen, len;
	fd_set		rset;
        struct sockaddr *cliaddr;
	const int 	on = 1;

	//since the server of well-known ports needs to run in root permission, so here I use the self-defined ports
	listentime = tcp_listen(NULL, "9500", &addrlen);
	listenecho = tcp_listen(NULL, "9501", &addrlen);

	//set up the socket option
	if(setsockopt(listentime, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		perror("setsockopt error...");
	if(setsockopt(listenecho, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		perror("setsockopt error...");
	
	cliaddr = malloc(addrlen);
	FD_ZERO(&rset);	
        while(1){
		len = addrlen;
		FD_SET(listentime, &rset);
		FD_SET(listenecho, &rset);
		maxfdp1 = max(listentime, listenecho) + 1;
		select(maxfdp1, &rset, NULL, NULL, NULL);

		iptr_time = malloc(sizeof(int));
		iptr_echo = malloc(sizeof(int));

		//deal with the daytime application-----------------------------------------------------------
		if(FD_ISSET(listentime, &rset)){
	        	if((*iptr_time = accept (listentime, cliaddr, &len)) < 0){
				perror("main: accept error...");
				continue;
			}
			else{
                		printf("connection from %s provided service of [DAYTIME]\n", sock_ntop (cliaddr, len) );
				pthread_create(&tid, NULL, &time_serv, iptr_time);
				continue;
			}
		}

		//deal with the echo application--------------------------------------------------------------
		if(FD_ISSET(listenecho, &rset)){
			if((*iptr_echo = accept (listenecho, cliaddr, &len)) < 0){
				perror("main: accept error...");
				continue;
			}
			else{
                		printf("connection from %s provided service of [ECHO]\n", sock_ntop (cliaddr, len) );
				pthread_create(&tid, NULL, &echo_serv, iptr_echo);
				continue;
			}
		}
	}
}

