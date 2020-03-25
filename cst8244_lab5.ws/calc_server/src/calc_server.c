#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "calc_message.h"
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
	int rcvid;        // indicates who we should reply to
	int chid;         // the channel ID
	client_send_t message;
	server_response_t reply;

	// create a channel --- Phase I
	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("failed to create the channel.");
		exit(EXIT_FAILURE);
	}

	// this is typical of a server:  it runs forever --- Phase II
	while (1) {

		rcvid = MsgReceive(chid, &message, sizeof(message),
		NULL);
		switch (message.operator) {
		case '/':
			if (message.right_hand == 0) {
				reply.statusCode = SRVR_UNDEFINED;
				sprintf(reply.errorMsg, "%s %d %c %d",
						"Error message from the server: UNDEFINED:",
						message.left_hand, message.operator,
						message.right_hand);
				break;
			}
			reply.statusCode = SRVR_OK;
			reply.answer = (float) message.left_hand / message.right_hand;
			break;
		case '+':
			if (message.right_hand > 0
					&& message.left_hand > INT_MAX - message.right_hand) {
				reply.statusCode = SRVR_OVERFLOW;
				sprintf(reply.errorMsg, "%s %d %c %d",
						"Error message from the server: OVERFLOW:",
						message.left_hand, message.operator,
						message.right_hand);
				break;
			}
			reply.statusCode = SRVR_OK;
			reply.answer = (double) message.left_hand
					+ (double) message.right_hand;
			break;
		case '-':
			reply.statusCode = SRVR_OK;
			reply.answer = (double) message.left_hand
					- (double) message.right_hand;
			break;
		case 'x':
			reply.statusCode = SRVR_OK;
			reply.answer = (double) message.left_hand
					* (double) message.right_hand;
			break;
		default:
			reply.statusCode = SRVR_INVALID_OPERATOR;
			sprintf(reply.errorMsg, "%s %c",
					"Error message from the server: INVALID OPERATOR:",
					message.operator);
		}

		MsgReply(rcvid, EOK, &reply, sizeof(reply));
	}

	// destroy the channel when done --- Phase III
	ChannelDestroy(chid);
	return EXIT_SUCCESS;
}
