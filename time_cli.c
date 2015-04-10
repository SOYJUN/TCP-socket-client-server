#include	"unp.h"

void day_cli(int sockfd){
	
	ssize_t nread;
	char 	recvline[MAXLINE];
	
	signal(SIGPIPE, SIG_IGN);
	while(1){
		if((nread = readline(sockfd, recvline, MAXLINE)) <= 0){
			snprintf(recvline, sizeof(recvline), "\n***str_cli: server terminated prematurely***\n");
			write(102, recvline, strlen(recvline)+1);
			exit(0);
		}

		if(write(102, recvline, strlen(recvline)+1) <= 0){
			if(errno == EINTR){
				snprintf(recvline, sizeof(recvline), "\n***Client termination: socket read returned with value -1, errno = EINTR***\n");
			write(102, recvline, strlen(recvline)+1);
			exit(0);
			}		     
		}	
	}
}

int main(int argc, char **argv){

	int	sockfd;
 	struct 	sockaddr_in servaddr;

     	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
     	servaddr.sin_family = AF_INET;
     	servaddr.sin_port = htons(9500);
     	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)	
			err_quit("inet_pton error for %s", argv[1]);

     	if(connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
		err_sys("connect error");

     	day_cli(sockfd);     /* do it all */

   	exit(0);
}
