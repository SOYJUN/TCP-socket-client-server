#include	"unp.h"

void str_cli(FILE *fp, int sockfd){
	
	ssize_t 	nread;
	char 		sendline[MAXLINE], recvline[MAXLINE];
	
	signal(SIGPIPE, SIG_IGN);
	while(fgets(sendline, MAXLINE, fp) != NULL){
		if(writen(sockfd, sendline, strlen(sendline)) < 0)
			perror("str_cli: write error");
		if(nread = (readline(sockfd, recvline, MAXLINE)) == 0){
			snprintf(recvline, sizeof(recvline), "\n***str_cli: server terminated prematurely***\n");
			write(101, recvline, strlen(recvline)+1);
			exit(0);
		}
		else if(nread < 0 && errno == EINTR){
			snprintf(recvline, sizeof(recvline), "\n***Client termination: socket read returned with value -1, errno = EINTR***\n");
			write(101, recvline, strlen(recvline)+1);
			exit(0);
		}
		else if(nread < 0)
			err_sys("echo_s: readline error");
	
		if(write(101, recvline, strlen(recvline)+1) <= 0)
			perror("write failed");
	}
}

int main(int argc, char **argv){

	int     sockfd;
 	struct sockaddr_in servaddr;

     	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
     	servaddr.sin_family = AF_INET;
     	servaddr.sin_port = htons(9501);
     	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)	
			err_quit("inet_pton_loose error");

     	if(connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
		err_sys("connect error");

     	str_cli(stdin, sockfd);     /* do it all */
   	exit(0);
}
