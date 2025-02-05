#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/siginfo.h>
#include <sys/stat.h>
#include <unistd.h>

/*function definitions*/
void sigint_handler(int);

volatile sig_atomic_t usr1Happened;

sem_t* sem;

void* childThread(void* args) {
	pthread_t currentThread;
	currentThread = pthread_self();
	int status;
	printf("Child thread %d created\n", currentThread);
	while (1) {
		status = sem_wait(sem);
		printf("Child thread %d unblocked\t Status: %d\n", currentThread, status);
		sleep(5);
	}
}

int main(void) {

	int numThreads, i;
	pthread_attr_t attr;


	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0; // or SA_RESTART

	//Setup sigaction for siguser1
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}

	printf("Enter the number of threads to create:\n");
	scanf(" %d", &numThreads);

	//Create semaphore
	sem = sem_open("Lab04_Semaphore", O_CREAT, S_IWOTH, 0);
	if (sem == SEM_FAILED) {
		printf("sem_open failed");
		return EXIT_FAILURE;
	}
	//create threads according to the number the user has requested
	for (i = 0; i < numThreads; i++) {
		pthread_attr_init(&attr);
		pthread_create(NULL, &attr, &childThread, NULL);
		pthread_attr_destroy(&attr);
	}
	while (usr1Happened != 1);

	i = sem_close(sem);
	if (i == -1) {
		printf("sem_close failed :[%s]\n", strerror(i));
		return EXIT_FAILURE;
	}
	//destroy named sem
	i = sem_unlink("Lab04_Semaphore");
	if (i == -1) {
		printf("sem_unlink failed :[%s]\n", strerror(i));
		return EXIT_FAILURE;
	}

	sleep(10);
	return EXIT_SUCCESS;
}

void sigint_handler(int sig) {
	usr1Happened = 1;
	return;
}
