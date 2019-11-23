CFLAGS=-Wall -Wextra -pedantic
CFLAGS+=-std=c11
CFLAGS+=-g

all: manager lpd client

manager: manager.o ports.o
lpd: lpd.o ports.o
client: client.o ports.o

clean:
	rm -f manager lpd client
	rm -f *.o
