#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
	puts("Hello World from QNX Neutrino RTOS!!!"); /* prints Hello World!!! */
	puts("@author Joseph Trottier (trot0097@algonquinlive.com)");
	puts("TODO: 'display something unique about myself' ");

	/*prints pid and parent pid*/
	printf("\nmy pid: %d  parent pid: %d", getpid(), getppid());

	/*keeps program running until user presses enter*/
	getchar();
	/*if the code gets here then it has completed
	 * it's job. return success*/
	return EXIT_SUCCESS;
}
