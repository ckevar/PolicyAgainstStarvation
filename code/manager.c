#include <sys/un.h>
#include <sys/socket.h>
#include <poll.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "ports.h"

#define N 5
#define BUFFER_SIZE 3
static bool allocated[N];
static int available = N;
static char printers[N][32];


void allocate(char *c) {
	unsigned int i;

	i = 0;
	while(allocated[i]) i++;

	allocated[i] = true;
	available--;
	sprintf(c, "/tmp/lpd%u", i);
}

void deallocate(const char *c)
{
	unsigned int i;
	int res;

	res = sscanf(c, "/tmp/lpd%u", &i);
	if ((res != 1) || i > N) {
		fprintf(stderr, "Strange deallocation %s: %d\n", c, i);
		return;
	}
	allocated[i] = false;
	available++;
}

struct 
{
	int reqRes[BUFFER_SIZE];
	bool toAllocate[BUFFER_SIZE];
	struct sockaddr_un sender[BUFFER_SIZE];
	int *atReq;
	struct sockaddr_un *atSender;
	int idx;
} buffer;

void initBuffer() {
	for (int i = 0; i < BUFFER_SIZE; ++i) {
		buffer.reqRes[i] = 0;
		buffer.toAllocate[i] = false;
	}
	buffer.atSender = buffer.sender;
	buffer.atReq = buffer.reqRes;
	buffer.idx = 0;
}

void lookForComplementsOfFirst() {
	int toServe = (N - available);

	for (int i = 0; i < buffer.idx; i++) {
		if ((toServe + buffer.reqRes[i]) <= N) {
			toServe += buffer.reqRes[i];
			buffer.toAllocate[i] = true;
		}
	}
}

void reorganizeBuffer() {
	int j = 0;	
	int tmpIdx = buffer.idx;
	for (int i = 0; i < tmpIdx; i++) {
		if (buffer.toAllocate[i]) {
			j++;
			buffer.idx--;
			buffer.atReq--;
			buffer.atSender--;
		} else {
			buffer.toAllocate[i - j] = buffer.toAllocate[i];
			buffer.reqRes[i - j] = buffer.reqRes[i];
			buffer.sender[i - j] = buffer.sender[i];
		}
	}
}

int main() {
	unsigned int i;
	int alloc_fd, dealloc_fd;
	int res;

	for (i = 0; i < N; i++) {
		sprintf(printers[i], "lpd%u", i);
		allocated[i] = false;
	}

	alloc_fd   = port_open("/tmp/Allocate_printer");
	dealloc_fd = port_open("/tmp/Deallocate_printer");

	initBuffer();

	while(1) {
		char msg[32];
		struct pollfd guards[2];

		guards[0].fd     = alloc_fd;
		guards[0].events = (buffer.idx < BUFFER_SIZE) ? POLLIN : 0;
		guards[1].fd     = dealloc_fd;
		guards[1].events = POLLIN;

		res = poll(guards, 2, -1);
		
		if (res <= 0)
			fprintf(stderr, "Error: poll() returned %d\n", res);

		struct sockaddr_un sender;
		socklen_t len;
		len = sizeof(struct sockaddr_un);

		printf("+++++++++++++++++++\n");
		if (guards[0].revents == POLLIN) {
			res = recvfrom(alloc_fd, msg, 32, 0, (struct sockaddr *) buffer.atSender, &len);
			
			if (res < 0) {
				perror("RecvFrom");
				return -1;
			}

			sscanf(msg, "%d", buffer.atReq);
			printf("reqRes: %d out of %u from %s\n", *buffer.atReq, available, buffer.atSender->sun_path);

			buffer.atReq++;
			buffer.atSender++;
			buffer.idx++;

		}


		if (guards[1].revents == POLLIN) {
			res = recvfrom(dealloc_fd, msg, 32, 0, (struct sockaddr *) &sender, &len);
			printf("Deallocating %s from %s, ", msg, sender.sun_path);

			if (res < 0) {
				perror("Recv");
				return -1;
			}

			deallocate(msg);
			printf(" available %u \n", available);
		}

		printf("Clients to be served are:\n");
		for (int i = 0; i < buffer.idx; i++) {
			printf("- %s reqs %d \n", buffer.sender[i].sun_path, buffer.reqRes[i]);
		}

		if (buffer.reqRes[0] <= available) {
			lookForComplementsOfFirst();
			for (int i = 0; i < buffer.idx; i++) {
				if (buffer.toAllocate[i]) {
					while(buffer.reqRes[i]--) {
						allocate(msg);
						printf("Allocated %s for %s\n", msg, buffer.sender[i].sun_path);
						res = sendto(alloc_fd, msg, strlen(msg) + 1, 0, (struct sockaddr *) &buffer.sender[i], len);
						if (res < 0) perror("SendTo");
					}
				}
			}
			reorganizeBuffer();
		}

	
	}

	return 0;
}
