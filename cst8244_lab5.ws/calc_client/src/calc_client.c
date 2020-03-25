#include <stdio.h>
#include <stdlib.h>
#include "..\..\calc_server\src\calc_message.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>



int main(int argc, char *argv[]) {

	if(argc != 5){
			printf("Usage: ./calc_client <Calc-Server-PID> left_operand operator right_operand");
			exit(EXIT_FAILURE);
		}

	client_send_t smsg;
	server_response_t rmsg;
	int  coid;
	pid_t serverpid;

	serverpid = atoi(argv[1]);

	// establish a connection --- Phase I
	coid = ConnectAttach (ND_LOCAL_NODE, serverpid, 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		printf("error, as calc_client can't connect attatched to processID %d (slogger:0)", serverpid);
		exit (EXIT_FAILURE);
	}
    smsg.left_hand = atoi(argv[2]);
    smsg.operator = argv[3][0];
    smsg.right_hand = atoi(argv[4]);

    // send the message --- Phase II
    // TODO (client) 1L
	if (MsgSend (coid, &smsg, sizeof(smsg), &rmsg, sizeof(rmsg)) == -1L) {
		fprintf (stderr, "MsgSend had an error.");
		exit (EXIT_FAILURE);
	}
	if(rmsg.statusCode == SRVR_OK)
		printf("The server has calculated the result of %d %c %d as %.2f",
				smsg.left_hand, smsg.operator, smsg.right_hand, rmsg.answer);
	else
		printf(rmsg.errorMsg);
	//Disconnect from the channel --- Phase III
	ConnectDetach(coid);
	return EXIT_SUCCESS;
}
