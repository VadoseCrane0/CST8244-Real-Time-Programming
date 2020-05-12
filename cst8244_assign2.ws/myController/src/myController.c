#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL
#define ATTACH_POINT "mydevice"

typedef union {
	struct _pulse pulse;
	char msg[255];
} my_message_t;

int main(void) {
	name_attach_t *attach;
	FILE *fd;
	int rcvid;
	my_message_t msg;
	char status[255], value[255];

	if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL) {
		return EXIT_FAILURE;
	}
	fd = fopen("/dev/local/mydevice", "r");
	fscanf(fd, "%s %s", status, value);
	//TODO: make this work
	if (strcmp(status, "status") == 0) {
		printf("Status: %s\n", value);
	}
	//TODO: make this work
	if (strcmp(value, "closed") == 0) {
		name_detach(attach, 0);
		return(EXIT_SUCCESS);
	}

	fclose(fd);

	while (1) {
		rcvid = MsgReceivePulse(attach->chid, &msg, sizeof(msg), NULL);
		if (rcvid == 0) {
			if (rcvid == MY_PULSE_CODE) {
				printf("Small Integer: %d\n", msg.pulse.value.sival_int);
				fd = fopen("/dev/local/mydevice", "r");
				//TODO: make this work
				fscanf(fd, "%s %s", status, value);
					//TODO: make this work
					if (strcmp(status, "status") == 0) {
						printf("Status: %s\n", value);
					}
					//TODO: make this work
					if (strcmp(value, "closed") == 0) {
						name_detach(attach, 0);
						return(EXIT_SUCCESS);
					}
				fclose(fd);
			}
		}else{
			printf("rcvid != 0");
			return(EXIT_FAILURE);
		}
	}
	name_detach(attach, 0);
	return EXIT_SUCCESS;
}
