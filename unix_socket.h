#ifndef SYSLOG_UNIX_SOCKET_H_
#define SYSLOG_UNIX_SOCKET_H_ 1

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

int unix_socket_open(const char* path, int socket_type);

void unix_socket_close(int socket);

ssize_t unix_socket_write(int socket, const struct iovec* iov, size_t iovcnt);

#endif /* SYSLOG_UNIX_SOCKET_H_ */
