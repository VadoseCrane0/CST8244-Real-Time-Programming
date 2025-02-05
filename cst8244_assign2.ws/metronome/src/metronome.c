#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <signal.h>

#define ATTACH_POINT "metronome"
#define METRONOME_PULSE_CODE _PULSE_CODE_MINAVAIL
#define PAUSE_PULSE (METRONOME_PULSE_CODE + 1)
#define SIZE_OF_T 8
#define QUIT_PULSE (METRONOME_PULSE_CODE + 2)

char data[255];
int server_coid;
typedef union {
	struct _pulse pulse;
	char msg[255];
} my_message_t;

int beatPerMinute, time_sig_top, time_sig_bottom;
double secs_per_beat;
long int nanoSecs;

struct DataTableRow {
	int time_sig_top;
	int time_sig_bottom;
	int interval_per_beat;
	char * interval;
}typedef DataTableRow;

DataTableRow t[SIZE_OF_T] = { { 2, 4, 4, "|1&2&" }, { 3, 4, 6, "|1&2&3&" }, { 4,
		4, 8, "|1&2&3&4&" }, { 5, 4, 10, "|1&2&3&4-5-" },
		{ 3, 8, 6, "|1-2-3-" }, { 6, 8, 6, "|1&a2&a" },
		{ 9, 8, 9, "|1&a2&a3&a" }, { 12, 8, 12, "|1&a2&a3&a4&a" } };

void* metronome_thread(void* arg) {
	//http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_lib_ref%2Ft%2Ftimer_create.html
	struct sigevent event;
	struct itimerspec timer;
	timer_t timer_id;
	name_attach_t * attach;
	int rcvid;
	int i, numDelay = 0, pause_flag = 0;

	my_message_t msg;
	if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL) {
		perror("name_attach did not work\n");
		exit(EXIT_FAILURE);
	}

	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, attach->chid,
	_NTO_SIDE_CHANNEL, 0);
	//If pid is zero, the current process is used to look up a nonzero tid. If pid and tid are zero, then the calling thread is used
	event.sigev_priority = SchedGet(0, 0, NULL);
	;
	event.sigev_code = METRONOME_PULSE_CODE;

	timer_create(CLOCK_REALTIME, &event, &timer_id);

	double beatPerSec = 60.0 / beatPerMinute;
	double measure = beatPerSec * time_sig_top;
	double interval = -1;
	for (i = 0; i < SIZE_OF_T; i++) {
		if (t[i].time_sig_top == time_sig_top
				&& t[i].time_sig_bottom == time_sig_bottom) {
			interval = measure / t[i].interval_per_beat;
			break;
		}
	}
	if (interval == -1) {
		printf("no data\n");
		exit(EXIT_FAILURE);
	}
	secs_per_beat = interval;

	timer.it_value.tv_sec = 1;
	timer.it_value.tv_nsec = 500000000;
	timer.it_interval.tv_sec = interval;
    nanoSecs = interval * 1e+9;
    timer.it_interval.tv_nsec = (nanoSecs);

	timer_settime(timer_id, 0, &timer, NULL);

	char *index = t[i].interval;
	for (;;) {
		rcvid = MsgReceivePulse(attach->chid, &msg, sizeof(msg), NULL);
		if (rcvid == 0) {
			switch (msg.pulse.code) {
			case METRONOME_PULSE_CODE:
				//if at the start print twice
				if (*index == '|') {
					printf("%c", *index++);
					printf("%c", *index++);
					fflush(stdout);
					//if at the end reset
				} else if (*index == '\0') {
					if (pause_flag == 1) {
						timer.it_value.tv_sec = numDelay;
						timer.it_value.tv_nsec = 500000000;
						timer.it_interval.tv_sec = interval;
						timer.it_interval.tv_nsec = nanoSecs;
						timer_settime(timer_id, 0, &timer, NULL);
						pause_flag = 0;
					}
					printf("\n");
					fflush(stdout);
					index = t[i].interval;
					//if at the middle print
				} else {
					printf("%c", *index++);
					fflush(stdout);
				}
				break;
			case PAUSE_PULSE:
				pause_flag = 1;
				numDelay = msg.pulse.value.sival_int;
				break;
			case QUIT_PULSE:
				printf("Quit");
				timer_delete(timer_id);
				exit(EXIT_SUCCESS);
				break;
			}
		}
	}
	name_detach(attach, 0);
}

int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {
	int nb;
	sprintf(data,
			"[metronome: %d beats/min, time signature %d/%d, secs-per-beat: %.2f, nanoSecs: %d]\n",
			beatPerMinute, time_sig_top, time_sig_bottom, secs_per_beat,
			nanoSecs);
	nb = strlen(data);

	if (data == NULL)
		return 0;

	//test to see if we have already sent the whole message.
	if (ocb->offset == nb)
		return 0;

	//We will return which ever is smaller the size of our data or the size of the buffer
	nb = min(nb, msg->i.nbytes);

	//Set the number of bytes we will return
	_IO_SET_READ_NBYTES(ctp, nb);

	//Copy data into reply buffer.
	SETIOV(ctp->iov, data, nb);

	//update offset into our data used to determine start position for next read.
	ocb->offset += nb;

	//If we are going to send any bytes update the access time for this resource.
	if (nb > 0)
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;

	return (_RESMGR_NPARTS(1));
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb) {
	int nb = 0;

	if (msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg))) {
		/* have all the data */
		char *buf;
		char *alert_msg;
		int i, small_integer;
		buf = (char *) (msg + 1);

		if (strstr(buf, "pause") != NULL) {
			for (i = 0; i < 2; i++) {
				alert_msg = strsep(&buf, " ");
			}
			small_integer = atoi(alert_msg);
			if (small_integer >= 1 && small_integer <= 9) {
				MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
				PAUSE_PULSE, small_integer);
			} else {
				printf("Integer is not between 1 and 9.\n");
			}
		} else if (strstr(buf, "quit") != NULL) {
			MsgSendPulse(server_coid, SchedGet(0, 0, NULL),
			QUIT_PULSE, 0);

		} else {
			printf("not a valid command\n");
		}

		nb = msg->i.nbytes;
	}
	_IO_SET_WRITE_NBYTES(ctp, nb);

	if (msg->i.nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS(0));
}

int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
		void *extra) {
	if ((server_coid = name_open(ATTACH_POINT, 0)) == -1) {
		perror("name_open failed.\n");
		return EXIT_FAILURE;
	}

	return (iofunc_open_default(ctp, msg, handle, extra));
}

int main(int argc, char *argv[]) {

//	setvbuf (stdout, NULL, _IOLBF, 0);
	dispatch_t* dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t connect_funcs;
	iofunc_attr_t ioattr;
	dispatch_context_t *ctp;
	int id;

	if (argc != 4) {
		printf("USAGE: ./metronome 120 2 4\n");
		exit(EXIT_FAILURE);
	}

	beatPerMinute = atoi(argv[1]);
	time_sig_top = atoi(argv[2]);
	time_sig_bottom = atoi(argv[3]);

	if ((dpp = dispatch_create()) == NULL) {
		fprintf(stderr, "%s:  Unable to allocate dispatch context.\n", argv[0]);
		return (EXIT_FAILURE);
	}
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS,
			&io_funcs);
	connect_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	iofunc_attr_init(&ioattr, S_IFCHR | 0666, NULL, NULL);

	if ((id = resmgr_attach(dpp, NULL, "/dev/local/metronome", _FTYPE_ANY, 0,
			&connect_funcs, &io_funcs, &ioattr)) == -1) {
		fprintf(stderr, "%s:  Unable to attach name.\n", argv[0]);
		return (EXIT_FAILURE);
	}
	ctp = dispatch_context_alloc(dpp);
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create( NULL, &attr, &metronome_thread, NULL);
	while (1) {
		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);
	}
	pthread_attr_destroy(&attr);
	return EXIT_SUCCESS;
}
