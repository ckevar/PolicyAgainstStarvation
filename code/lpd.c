#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "ports.h"

int main(int argc, char *argv[])
{
	int s;
	char port_name[32];

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <id>\n", argv[0]);

		return -1;
	}
	sprintf(port_name, "/tmp/lpd%u", atoi(argv[1]));
	s = port_open(port_name);
	if (s < 0) {
		return -1;
	}

	while(1) {
		struct sockaddr_un sender;
		socklen_t len;
		char doc[64];
		int res;
		unsigned int j;

		len = sizeof(struct sockaddr_un);
		res = recvfrom(s, doc, 64, 0, (struct sockaddr *) &sender, &len);
		if (res < 0) {
	  		perror("RecvFrom");
			return -1;
		}
		printf("Received document to print: %s\n", doc);
		printf("Printing");
		for (j = 0; j < 10; j++) {
			fflush(stdout);
			printf(".");
			sleep(1);
		}
		printf("\n");
		res = sendto(s, "DONE", 4, 0, (struct sockaddr *) &sender, len);
		if (res < 0) { 
			perror("SendTo");
		}
  }
}
