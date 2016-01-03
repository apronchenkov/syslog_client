#include "syslog_client.h"
#include "syslog_client_ext.h"
#include "syslog_local_transport.h"
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

struct syslog_client {
  syslog_message_format message_format;
  int facility;
  size_t tag_size;
  char *tag;
  size_t hostname_size;
  char *hostname;
  syslog_transport *transport;
};

static char *hostname() {
  size_t buffer_size = 8;
  char *buffer = NULL;
  do {
    free(buffer);
    if (buffer_size > buffer_size + buffer_size) {
      errno = EOVERFLOW;
      return NULL; /* Integer overflow */
    }
    buffer_size = buffer_size + buffer_size;
    buffer = malloc(buffer_size);
    if (!buffer) {
      return NULL;
    }
  } while (gethostname(buffer, buffer_size) != 0);
  return buffer;
}

syslog_client *syslog_open(int facility, const char *tag) {
  syslog_transport *transport = syslog_local_transport_create();
  if (transport) {
    return syslog_open_ext(SYSLOG_MESSAGE_FORMAT_LOCAL, transport, facility,
                           tag);
  }
  return NULL;
}

syslog_client *syslog_open_ext(syslog_message_format message_format,
                               syslog_transport *transport, int facility,
                               const char *tag) {
  int errno_copy;
  syslog_client *self = malloc(sizeof(syslog_client));
  if (!self) {
    goto on_fail;
  }
  self->message_format = message_format;
  self->facility = facility;
  self->tag = NULL;
  self->hostname = NULL;
  self->transport = transport;
  self->tag = strdup(tag);
  if (!self->tag) {
    goto on_fail;
  }
  self->tag_size = strlen(self->tag);
  self->hostname = hostname();
  if (!self->hostname) {
    goto on_fail;
  }
  self->hostname_size = strlen(self->hostname);
  return self;
on_fail:
  errno_copy = errno;
  syslog_close(self);
  errno = errno_copy;
  return NULL;
}

void syslog_close(syslog_client *self) {
  if (self) {
    free(self->tag);
    free(self->hostname);
    if (self->transport) {
      self->transport->destroy(self->transport);
    }
    free(self);
  }
}

static bool syslog_send_ext_local(syslog_client *self, int serverity,
                                  struct timeval *tv, const char *message,
                                  size_t message_size) {
  struct tm tm;
  localtime_r(&tv->tv_sec, &tm);
  char header[32];
  size_t header_size = 0;
  header_size += snprintf(header + header_size, sizeof(header) - header_size,
                          "<%d>", self->facility | serverity);
  header_size += strftime(header + header_size, sizeof(header) - header_size,
                          "%h %e %T ", &tm);
  char pid[32];
  size_t pid_size = snprintf(pid, sizeof(pid), "[%lu]: ", (long)getpid());
  size_t iovcnt = 0;
  struct iovec iov[4];
  iov[iovcnt].iov_base = header;
  iov[iovcnt].iov_len = header_size;
  ++iovcnt;
  iov[iovcnt].iov_base = self->tag;
  iov[iovcnt].iov_len = self->tag_size;
  ++iovcnt;
  iov[iovcnt].iov_base = pid;
  iov[iovcnt].iov_len = pid_size;
  ++iovcnt;
  iov[iovcnt].iov_base = (void *)message;
  iov[iovcnt].iov_len = message_size;
  ++iovcnt;
  assert(iovcnt <= sizeof(iov) / sizeof(iov[0]));
  return self->transport->send(self->transport, iov, iovcnt);
}

static bool syslog_send_ext_remote(syslog_client *self, int serverity,
                                   struct timeval *tv, const char *message,
                                   size_t message_size) {
  struct tm tm;
  localtime_r(&tv->tv_sec, &tm);
  char header[32];
  size_t header_size = 0;
  header_size += snprintf(header + header_size, sizeof(header) - header_size,
                          "<%d>", self->facility | serverity);
  header_size += strftime(header + header_size, sizeof(header) - header_size,
                          "%h %e %T ", &tm);
  char pid[32];
  size_t pid_size = snprintf(pid, sizeof(pid), "[%lu]: ", (long)getpid());
  size_t iovcnt = 0;
  struct iovec iov[6];
  iov[iovcnt].iov_base = header;
  iov[iovcnt].iov_len = header_size;
  ++iovcnt;
  iov[iovcnt].iov_base = self->hostname;
  iov[iovcnt].iov_len = self->hostname_size;
  ++iovcnt;
  iov[iovcnt].iov_base = " ";
  iov[iovcnt].iov_len = 1;
  ++iovcnt;
  iov[iovcnt].iov_base = self->tag;
  iov[iovcnt].iov_len = self->tag_size;
  ++iovcnt;
  iov[iovcnt].iov_base = pid;
  iov[iovcnt].iov_len = pid_size;
  ++iovcnt;
  iov[iovcnt].iov_base = (void *)message;
  iov[iovcnt].iov_len = message_size;
  ++iovcnt;
  assert(iovcnt <= sizeof(iov) / sizeof(iov[0]));
  return self->transport->send(self->transport, iov, iovcnt);
}

bool syslog_send_ext(syslog_client *self, int serverity, struct timeval *tv,
                     const char *message, size_t message_size) {
  if (self && self->transport) {
    if (self->message_format == SYSLOG_MESSAGE_FORMAT_LOCAL) {
      return syslog_send_ext_local(self, serverity, tv, message, message_size);
    } else if (self->message_format == SYSLOG_MESSAGE_FORMAT_REMOTE) {
      return syslog_send_ext_remote(self, serverity, tv, message, message_size);
    }
  }
  return false;
}

bool syslog_send(syslog_client *self, int serverity, const char *format, ...) {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) == -1) {
    return false;
  }
  char message[128];
  va_list args;
  va_start(args, format);
  const int n = vsnprintf(message, sizeof(message), format, args);
  va_end(args);
  if (n < 0) {
    errno = EINVAL; /* FIXME: What is the right code? */
    return false;
  }
  if ((size_t)n < sizeof(message)) {
    return syslog_send_ext(self, serverity, &tv, message, (size_t)n);
  }
  char *big_message = malloc(n + 1);
  if (big_message == NULL) {
    return false;
  }
  va_start(args, format);
  vsnprintf(big_message, n + 1, format, args);
  va_end(args);
  const bool result =
      syslog_send_ext(self, serverity, &tv, big_message, (size_t)n);
  const int errno_copy = errno;
  free(big_message);
  errno = errno_copy;
  return result;
}
