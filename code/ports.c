#include <sys/un.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "ports.h"

int port_open(const char *name)
{
  int res, err;
  struct sockaddr_un port_addr;

  res = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (res < 0) {
    perror("Socket");

    return -1;
  }

  unlink(name);
  memset(&port_addr, 0, sizeof(struct sockaddr_un));
  port_addr.sun_family = AF_UNIX;
  strncpy(port_addr.sun_path, name, sizeof(port_addr.sun_path) - 1);
  err = bind(res, (struct sockaddr *) &port_addr, sizeof(struct sockaddr_un));
  if (err < 0) {
    perror("Bind");

    return -1;
  }

  return res;
}

int port_send(int s, const char *msg, int size, const char *dst_addr)
{
  struct sockaddr_un dst;
  socklen_t len;
  int res;

  memset(&dst, 0, sizeof(struct sockaddr_un));
  dst.sun_family = AF_UNIX;
  strncpy(dst.sun_path, dst_addr, sizeof(dst.sun_path) - 1);
  len = sizeof(struct sockaddr_un);

  res = sendto(s, msg, size, 0, (struct sockaddr *)&dst, len);
  if (res < 0) {
    perror("SendTo ");
  }

  return res;
}

