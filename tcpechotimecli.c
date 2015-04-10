#include	"unp.h"

#define PSIZE 1024

void sig_chld(int signo){
	pid_t pid;
	int stat;
	while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("***child %d of client terminated!***\n", pid);
	return;
}

int main(int argc, char **argv){

	int     pfd[2], nread, i;
	char    choice[1024], str[INET_ADDRSTRLEN], **pptr, buf[PSIZE];
	pid_t 	pid;
	struct  hostent *hptr;	

	if (argc != 2)
		err_quit("Error: please enter <IPaddress>");
	
	//get host by name----------------------------------------------------------------------------------
	if(('a'<argv[1][0] && argv[1][0]<'z') || ( 'A'<argv[1][0] && argv[1][0]<'Z')){
		if((hptr = gethostbyname(argv[1])) == NULL){
			err_msg("gethostbyname error for host: %s:%s", argv[1], hstrerror(h_errno));
		}

		switch(hptr -> h_addrtype){
		case AF_INET:    
			pptr = hptr -> h_addr_list;
			printf("\nthe IP address of %s: ", hptr -> h_name);
			inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
			printf("\t%s\n", str);
			break;
		default:
			err_ret("unknown address type");
			break;
		}
                for(i = 0; i < sizeof(str); i++)
                        argv[1][i] = str[i];
	}
	//----------------------------------------------------------------------------------------------------
	//get host by address---------------------------------------------------------------------------------
	else{		
		in_addr_t addr = inet_addr(argv[1]);
		if((hptr = gethostbyaddr((const char *)&addr, 4, AF_INET)) == NULL)
			err_msg("gethostbyaddr error for host: %s:%s", argv[1], hstrerror(h_errno));
		printf("\nthe server host of %s: ", argv[1]);
		printf("\t%s\n", hptr -> h_name);
	}	
	//----------------------------------------------------------------------------------------------------
	// infinite loop in which it queries the user which service is being requested------------------------
	 
 	signal(SIGCHLD, sig_chld);

	while(1){
		printf("\n--------------------------------------------------\n");
		puts("Please select your prefer service:\n  1. Echo\n  2. Time\n  3. Quit\nEnter your choice: ");
	
		if(fgets(choice, 1024, stdin) == NULL){
			if(errno == EINTR){
				perror("fgets error");
				printf("restart...");
				continue;
			}
			else{
				perror("fgets else error");
				break;
			}
		}                             

		if(bcmp(choice, "1", 1) && bcmp(choice, "2", 1) && bcmp(choice, "3", 1)){ 
			printf("Please enter a valid number from 1 to 3\n");
			continue;	
		}
		
		//Echo-----------------------------------------------------------------------------------------
		if(!bcmp(choice, "1", 1)){
			printf("Your choice is Echo service\n");				

			if (pipe(pfd) == -1)
    				perror("pipe failed");

			//child process-------------------------------------------------------
  			if ((pid = fork()) == 0){
    				close(pfd[0]);
				dup2(pfd[1], 101);
				close(pfd[1]);
				if((execlp("xterm", "xterm", "-e", "./echo_cli", argv[1], (char *) 0)) < 0)
					err_quit("execlp fail");
				exit(0);
			}
			//parent process------------------------------------------------------
			else{
				close(pfd[1]);
				dup2(pfd[0], 100);
				close(pfd[0]);
				while((nread = read(100, buf, PSIZE)) != 0)
					printf("[Echo]: %s", buf);
			}			
		}

		//Time-----------------------------------------------------------------------------------------
		else if(!bcmp(choice, "2", 1)){
			printf("Your choice is Time service\n");

			if (pipe(pfd) == -1)
    				perror("pipe failed");
    
			//child process-------------------------------------------------------
  			if ((pid = fork()) == 0){
    				close(pfd[0]);
				dup2(pfd[1], 102);
				close(pfd[1]);
				if((execlp("xterm", "xterm", "-e", "./time_cli", argv[1], (char *) 0)) < 0)
					err_quit("execlp fail");
				exit(0);
			}
			//parent process------------------------------------------------------
			else{
				close(pfd[1]);
				dup2(pfd[0], 100);
				close(pfd[0]);
				while((nread = read(100, buf, PSIZE)) != 0)
					printf("[Daytime]: %s", buf);	
			}
		}
			
		//Quit-----------------------------------------------------------------------------------------
		else if(!bcmp(choice, "3", 1)){
			printf("Your choice is to Quit the program\n");
			exit(0);
		}
		continue;
	}
} 


