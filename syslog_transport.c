#include "syslog_transport.h"
#include "unix_socket.h"
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct syslog_transport_default syslog_transport_default;

struct syslog_transport_default {
  syslog_transport base;
  pthread_mutex_t lock;
  int socket_type;
  int socket;
};

static void syslog_transport_default_close(syslog_transport_default *self) {
  if (!self->socket_type) {
    unix_socket_close(self->socket);
    self->socket_type = 0;
    self->socket = -1;
  }
}

static void syslog_transport_default_destroy(syslog_transport_default *self) {
  pthread_mutex_destroy(&self->lock);
  syslog_transport_default_close(self);
  free(self);
}

static bool syslog_transport_default_reconnect(syslog_transport_default *self) {
  static const char *PATHS[] = {"/dev/log", "/var/run/syslog", "/var/run/log",
                                NULL};
  static const int TYPES[] = {SOCK_DGRAM, SOCK_STREAM, 0};
  syslog_transport_default_close(self);
  for (int i = 0; PATHS[i]; ++i) {
    for (int j = 0; TYPES[j]; ++j) {
      const int socket = unix_socket_open(PATHS[i], TYPES[j]);
      if (socket != -1) {
        self->socket = socket;
        self->socket_type = TYPES[j];
        return true;
      }
    }
  }
  return false;
}

static void iovec_advance(struct iovec *iov, size_t *iovcnt, size_t offset) {
  size_t i = 0, j;
  for (j = 0; j < *iovcnt; ++j) {
    if (offset >= iov[j].iov_len) {
      offset -= iov[j].iov_len;
    } else {
      iov[i].iov_base = (char *)iov[j].iov_base + offset;
      iov[i].iov_len = iov[j].iov_len - offset;
      offset = 0;
      ++i;
    }
  }
  *iovcnt = i;
}

static bool syslog_transport_default_send_impl(syslog_transport_default *self,
                                               const struct iovec *iov,
                                               size_t iovcnt) {
  if (self->socket_type == SOCK_DGRAM) {
    return unix_socket_write(self->socket, iov, iovcnt) != -1;
  } else if (self->socket_type == SOCK_STREAM) {
    struct iovec iov_local[iovcnt + 1];
    size_t iovcnt_local = iovcnt + 1;
    memcpy(&iov_local, iov, iovcnt * sizeof(struct iovec));
    iov_local[iovcnt].iov_base = "";
    iov_local[iovcnt].iov_len = 1;
    while (iovcnt_local > 0) {
      const ssize_t send_ret =
          unix_socket_write(self->socket, iov_local, iovcnt_local);
      if (send_ret == -1) {
        return false;
      }
      iovec_advance(iov_local, &iovcnt_local, send_ret);
    }
    return true;
  } else {
    errno = EBADF;
    return false;
  }
}

bool syslog_transport_default_send(syslog_transport_default *self,
                                   const struct iovec *iov, size_t iovcnt) {
  pthread_mutex_lock(&self->lock);
  if (syslog_transport_default_send_impl(self, iov, iovcnt) ||
      (syslog_transport_default_reconnect(self) &&
       syslog_transport_default_send_impl(self, iov, iovcnt))) {
    pthread_mutex_unlock(&self->lock);
    return true;
  } else {
    const int errno_copy = errno;
    syslog_transport_default_close(self); /* drop current state */
    pthread_mutex_unlock(&self->lock);
    errno = errno_copy;
    return false;
  }
}

syslog_transport *syslog_transport_create_default() {
  syslog_transport_default *self = malloc(sizeof(syslog_transport_default));
  if (!self) {
    return NULL;
  }
  self->base.send = (syslog_transport_send_func)&syslog_transport_default_send;
  self->base.destroy =
      (syslog_transport_destroy_func)&syslog_transport_default_destroy;
  self->socket_type = 0;
  self->socket = -1;
  if (!syslog_transport_default_reconnect(self) ||
      pthread_mutex_init(&self->lock, NULL) != 0) {
    int errno_copy = errno;
    free(self);
    errno = errno_copy;
    return NULL;
  }
  return (syslog_transport *)self;
}
