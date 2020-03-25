#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

void sigint_handler(int);


	volatile sig_atomic_t usr1Happened=0;


int main(void) {

	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0; // or SA_RESTART
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
			perror("sigaction");
			exit(1);
	}

	printf("PID = %d : Running...\n", getpid());
	while(!usr1Happened);
	printf("PID = %d : Received Exiting.\n", getpid());

	return EXIT_SUCCESS;
}

void sigint_handler(int sig) {
	usr1Happened = !usr1Happened;
	printf("PID = %d : Received USR1.\n", getpid());
}
