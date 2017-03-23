#include "SyslogTransport.h"
#include "UnixSocket.h"
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct SyslogTransportDefault SyslogTransportDefault;

struct SyslogTransportDefault {
  SyslogTransport base;
  pthread_mutex_t lock;
  int socketType;
  int socket;
};

static void SyslogTransportDefaultClose(SyslogTransportDefault* self) {
  if (!self->socketType) {
    UnixSocketClose(self->socket);
    self->socketType = 0;
    self->socket = -1;
  }
}

static void SyslogTransportDefaultDestroy(SyslogTransportDefault* self) {
  pthread_mutex_destroy(&self->lock);
  SyslogTransportDefaultClose(self);
  free(self);
}

static bool SyslogTransportDefaultReconnect(SyslogTransportDefault* self) {
  static const char* PATHS[] = {"/dev/log", "/var/run/syslog", "/var/run/log",
                                NULL};
  static const int TYPES[] = {SOCK_DGRAM, SOCK_STREAM, 0};
  SyslogTransportDefaultClose(self);
  for (int i = 0; PATHS[i]; ++i) {
    for (int j = 0; TYPES[j]; ++j) {
      const int socket = UnixSocketOpen(PATHS[i], TYPES[j]);
      if (socket != -1) {
        self->socket = socket;
        self->socketType = TYPES[j];
        return true;
      }
    }
  }
  return false;
}

static void IoVecAdvance(struct iovec* iov, size_t* iovcnt, size_t offset) {
  size_t i = 0, j;
  for (j = 0; j < *iovcnt; ++j) {
    if (offset >= iov[j].iov_len) {
      offset -= iov[j].iov_len;
    } else {
      iov[i].iov_base = (char*)iov[j].iov_base + offset;
      iov[i].iov_len = iov[j].iov_len - offset;
      offset = 0;
      ++i;
    }
  }
  *iovcnt = i;
}

static bool SyslogTransportDefaultSendImpl(SyslogTransportDefault* self,
                                           const struct iovec* iov,
                                           size_t iovcnt) {
  if (self->socketType == SOCK_DGRAM) {
    return UnixSocketWrite(self->socket, iov, iovcnt) != -1;
  } else if (self->socketType == SOCK_STREAM) {
    struct iovec iovLocal[iovcnt + 1];
    size_t iovcntLocal = iovcnt + 1;
    memcpy(&iovLocal, iov, iovcnt * sizeof(struct iovec));
    iovLocal[iovcnt].iov_base = "";
    iovLocal[iovcnt].iov_len = 1;
    while (iovcntLocal > 0) {
      const ssize_t send_ret =
          UnixSocketWrite(self->socket, iovLocal, iovcntLocal);
      if (send_ret == -1) {
        return false;
      }
      IoVecAdvance(iovLocal, &iovcntLocal, send_ret);
    }
    return true;
  } else {
    errno = EBADF;
    return false;
  }
}

bool SyslogTransportDefaultSend(SyslogTransportDefault* self,
                                const struct iovec* iov, size_t iovcnt) {
  pthread_mutex_lock(&self->lock);
  if (SyslogTransportDefaultSendImpl(self, iov, iovcnt) ||
      (SyslogTransportDefaultReconnect(self) &&
       SyslogTransportDefaultSendImpl(self, iov, iovcnt))) {
    pthread_mutex_unlock(&self->lock);
    return true;
  } else {
    const int errnoCopy = errno;
    SyslogTransportDefaultClose(self); /* drop current state */
    pthread_mutex_unlock(&self->lock);
    errno = errnoCopy;
    return false;
  }
}

SyslogTransport* SyslogTransportCreateDefault() {
  SyslogTransportDefault* self = malloc(sizeof(SyslogTransportDefault));
  if (!self) {
    return NULL;
  }
  self->base.send = (SyslogTransportSendFunc)&SyslogTransportDefaultSend;
  self->base.destroy =
      (SyslogTransportDestroyFunc)&SyslogTransportDefaultDestroy;
  self->socketType = 0;
  self->socket = -1;
  if (!SyslogTransportDefaultReconnect(self) ||
      pthread_mutex_init(&self->lock, NULL) != 0) {
    int errnoCopy = errno;
    free(self);
    errno = errnoCopy;
    return NULL;
  }
  return (SyslogTransport*)self;
}
