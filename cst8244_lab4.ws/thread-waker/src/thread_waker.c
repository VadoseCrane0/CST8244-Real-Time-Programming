#include <process.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

sem_t* sem;

int main(void) {
	int numWakeup = 1, i;
	sem = sem_open("Lab04_Semaphore", 0);
	printf("thread-waker PID: %d\n", getpid());
	do {
		printf("How many threads do you want to wake-up(0 to exit)?\n");
		scanf("%d", &numWakeup);
		for (i = 0; i < numWakeup; i++) {
			sem_post(sem);
		}
	} while (numWakeup != 0);

	return EXIT_SUCCESS;
}
