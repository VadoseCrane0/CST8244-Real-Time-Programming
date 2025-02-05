#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <errno.h>
#include "des.h"

int main() {
	int displayPid = getpid();
	printf("The display is running as PID: %d\n", displayPid);
	int rcvid;        // indicates who we should reply to
	int chid;         // the channel ID
	int breakLoop = FALSE; //flag to exit while loop Exit
	int reply = 0;
	Display message;
	// create a channel --- Phase I
	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("failed to create the channel.");
		exit(EXIT_FAILURE);
	}
	// this is typical of a server:  it runs forever --- Phase II
	while (1) {
		if (breakLoop == TRUE)
			break;
		rcvid = MsgReceive(chid, &message, sizeof(message), NULL);
		if (rcvid == -1) {
			perror("failed to receive message from controller");
			exit(EXIT_FAILURE);
		}
		MsgReply(rcvid, EOK, &reply, sizeof(reply));

		switch (message.outputMessage) {
		case EXIT:
			printf("%s\n", outMessage[EXIT]);
			breakLoop = TRUE;
			break; // runs forever until exit sets flag to end
		case DISPLAY_PERSON_ID:
			printf("%s%d\n", outMessage[DISPLAY_PERSON_ID], message.person.id);
			break;
		case DISPLAY_PERSON_WEIGHT:
			printf("%s%d\n", outMessage[DISPLAY_PERSON_WEIGHT], message.person.weight);
			break;
		default:
			printf("%s\n", outMessage[message.outputMessage]);
			break;
		}
	}

	// destroy the channel when done --- Phase III
	if (ChannelDestroy(chid) == -1) {
		printf("Failed to destroy channel");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
