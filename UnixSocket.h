#pragma once
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

int UnixSocketOpen(const char *path, int socket_type);

void UnixSocketClose(int socket);

ssize_t UnixSocketWrite(int socket, const struct iovec *iov, size_t iovcnt);
