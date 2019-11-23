#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include "ports.h"

int leaveWhile;

#define NUM_RESOURCES 5

void callback() {
	leaveWhile = 0;
}

int main(int argc, char const *argv[])
{

	int s;
	int reqResCurr = 3;
	int reqRes;
	ssize_t res;
	ssize_t deallocation;
	char m[64];
	char my_name[32];
	char lpd_id[32];
	int pid = getpid();
	int computationTime = 0;

	const char *REQ_ALLOC = "/tmp/Allocate_printer";
	const char *REQ_DEALLOC = "/tmp/Deallocate_printer";

	srand(time(NULL));

	sprintf(my_name, "/tmp/client%d", pid);
	s = port_open(my_name);
	if (s < 0) 
		return -1;

	leaveWhile = 1;
	signal(SIGINT, callback);

	struct sockaddr_un sender;
	socklen_t len;
	len = sizeof(struct sockaddr_un);

	while(leaveWhile) {
		if (argc < 2) {
			reqResCurr = rand() % (NUM_RESOURCES - 1) + 1;
		} else {
			reqResCurr = atoi(argv[1]);
		}

		printf("INIT SESSION %d, %d to alloc\n", pid, reqResCurr);
		reqRes = reqResCurr;
		deallocation = 0; 
		
		sprintf(m, "%u", reqRes);
		res = port_send(s, m, 2, REQ_ALLOC);
		if (res < 0) return -1;

		while(reqRes) {
			res = recvfrom(s, lpd_id, 32, 0, (struct sockaddr *) &sender, &len);
			if (res < 0) {
				perror("Recv");
				break;
			}

			if (strcmp(sender.sun_path, REQ_ALLOC) == 0) {
				printf("ALLOC %s \n", lpd_id);
				sprintf(m, "Message %d", pid);
				res = port_send(s, m, strlen(m) + 1, lpd_id);

				if (res < 0) {
					port_send(s, lpd_id, strlen(lpd_id) + 1, REQ_DEALLOC);
					return -1;
				}

			} else {
				printf("DEALLOC of %s ", sender.sun_path);
				deallocation = port_send(s, sender.sun_path, strlen(sender.sun_path), REQ_DEALLOC);
				if (deallocation < 0) {
					printf("Couldnt deallocated \n");
				}
				--reqRes;
				printf("succefull, %d to go \n", reqRes);
			}
		}	
		/* DO SOMETHING */
		computationTime = rand() % 10 + 1;
		printf("Next event in %d seconds\n", computationTime);
		sleep(computationTime);
		/***************/
	}
	return 0;
}
