#include "UnixSocket.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static bool UnixSocketConnect(int socket, const void *address,
                              socklen_t address_len) {
  bool ok;
  do {
    ok = (connect(socket, (const struct sockaddr *)address, address_len) == 0);
  } while (!ok && errno == EINTR);
  return ok;
}

static bool UnixSocketSetCloexec(int socket) {
  int flags;
  bool ok;
  do {
    flags = fcntl(socket, F_GETFD);
  } while (flags == -1 && errno == EINTR);
  if (flags == -1) {
    return false;
  }
  do {
    ok = (fcntl(socket, F_SETFD, flags | FD_CLOEXEC) == 0);
  } while (!ok && errno == EINTR);
  return ok;
}

int UnixSocketOpen(const char *path, int socket_type) {
  int sock = -1;
  struct sockaddr_un sockaddr_un;
  if (sizeof(sockaddr_un.sun_path) <= strlen(path)) {
    errno = ENAMETOOLONG;
    goto on_fail;
  }
  memset(&sockaddr_un, 0, sizeof(sockaddr_un));
  sockaddr_un.sun_family = AF_UNIX;
  strcpy(sockaddr_un.sun_path, path);
  sock = socket(AF_UNIX, socket_type, 0);
  if (sock == -1) {
    goto on_fail;
  }
  if (!UnixSocketConnect(sock, &sockaddr_un, sizeof(sockaddr_un))) {
    goto on_fail;
  }
  if (!UnixSocketSetCloexec(sock)) {
    goto on_fail;
  }
  return sock;
on_fail:
  if (sock != -1) {
    const int errno_copy = errno;
    UnixSocketClose(sock);
    errno = errno_copy;
  }
  return -1;
}

void UnixSocketClose(int socket) {
  bool ok;
  do {
    ok = (close(socket) == 0);
  } while (!ok && errno == EINTR);
  if (!ok && errno != EBADF) {
    /* TODO: Sleep? */
    do {
      ok = (close(socket) == 0);
    } while (!ok && errno == EINTR);
  }
}

ssize_t UnixSocketWrite(int socket, const struct iovec *iov, size_t iovcnt) {
  struct msghdr message;
  ssize_t result;
  memset(&message, 0, sizeof(message));
  message.msg_iov = (struct iovec *)iov;
  message.msg_iovlen = iovcnt;
  do {
    result = sendmsg(socket, &message, 0);
  } while (result == -1 && errno == EINTR);
  return result;
}
