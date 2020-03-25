#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>

/*function definitions*/
void sigint_handler(int);

volatile sig_atomic_t usr1Happened;
pid_t pid;
pid_t ppid;
int main(void) {
	ppid = getpid();
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0; // or SA_RESTART
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
				perror("sigaction");
				exit(1);
	}

	int rv,numChildren,i;

	printf("Enter the number of children:");
	/* get and error check number of children to create*/
	scanf(" %d", &numChildren);
	printf("%d\n\n", numChildren);
	printf("PID = %d : Parent running...\n", getpid());
	for(i=0; i< numChildren; i++)
		switch (pid = fork()) {
			case -1:
				perror("fork"); /* something went wrong */
				exit(1); /* parent exits */
			case 0:
				/* we are the child now*/
				printf("PID = %d : Child running...\n", getpid());
				while(usr1Happened != 1);
				exit(rv);
			}
	while(numChildren != 0)
		if(wait(&rv) > 0)
			numChildren--;

	printf("PID = %d : Children finished, parent exiting.\n", getpid());
	exit(EXIT_SUCCESS);
}

void sigint_handler(int sig) {
	printf("handler\n");
	if(getpid() == ppid)return;
	printf("PID = %d : Child Received USR1.\n", getpid());
	printf("PID = %d : Child exiting.\n", getpid());
	usr1Happened = 1;
}
