#pragma once
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

int unix_socket_open(const char* path, int socket_type);

void unix_socket_close(int socket);

ssize_t unix_socket_write(int socket, const struct iovec* iov, size_t iovcnt);
