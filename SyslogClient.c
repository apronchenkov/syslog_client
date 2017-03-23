#define _BSD_SOURCE
#include "SyslogClient.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

struct SyslogClient {
  SyslogMessageFormat messageFormat;
  int facility;
  size_t tagSize;
  char* tag;
  size_t hostnameSize;
  char* hostname;
  SyslogTransport* transport;
};

static char* hostname() {
  size_t bufferSize = 8;
  char* buffer = NULL;
  do {
    free(buffer);
    if (bufferSize > bufferSize + bufferSize) {
      errno = EOVERFLOW;
      return NULL; /* Integer overflow */
    }
    bufferSize = bufferSize + bufferSize;
    buffer = malloc(bufferSize);
    if (!buffer) {
      return NULL;
    }
  } while (gethostname(buffer, bufferSize) != 0);
  return buffer;
}

SyslogClient* SyslogClientCreateDefault(int facility, const char* tag) {
  SyslogTransport* transport = SyslogTransportCreateDefault();
  if (transport) {
    return SyslogClientCreate(SYSLOG_MESSAGE_FORMAT_LOCAL, transport, facility,
                              tag);
  }
  return NULL;
}

SyslogClient* SyslogClientCreate(SyslogMessageFormat messageFormat,
                                 SyslogTransport* transport, int facility,
                                 const char* tag) {
  SyslogClient* self = malloc(sizeof(SyslogClient));
  if (!self) {
    const int errno_copy = errno;
    SyslogTransportDestroy(transport);
    errno = errno_copy;
    return NULL;
  }
  self->messageFormat = messageFormat;
  self->facility = facility;
  self->tag = NULL;
  self->hostname = NULL;
  self->transport = transport;
  self->tag = strdup(tag);
  if (!self->tag) {
    const int errnoCopy = errno;
    SyslogClientDestroy(self);
    errno = errnoCopy;
    return NULL;
  }
  self->tagSize = strlen(self->tag);
  self->hostname = hostname();
  if (!self->hostname) {
    const int errnoCopy = errno;
    SyslogClientDestroy(self);
    errno = errnoCopy;
    return NULL;
  }
  self->hostnameSize = strlen(self->hostname);
  return self;
}

void SyslogClientDestroy(SyslogClient* self) {
  if (self) {
    free(self->tag);
    free(self->hostname);
    SyslogTransportDestroy(self->transport);
    free(self);
  }
}

bool SyslogClientPrintf(SyslogClient* self, int severity, const char* format,
                        ...) {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) == -1) {
    return false;
  }
  const double unixTime = tv.tv_sec + 1e-6 * tv.tv_usec;
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
    return SyslogClientSend(self, severity, unixTime, message, (size_t)n);
  }
  char* bigMessage = malloc(n + 1);
  if (bigMessage == NULL) {
    return false;
  }
  va_start(args, format);
  vsnprintf(bigMessage, n + 1, format, args);
  va_end(args);
  const bool result =
      SyslogClientSend(self, severity, unixTime, bigMessage, (size_t)n);
  const int errnoCopy = errno;
  free(bigMessage);
  errno = errnoCopy;
  return result;
}

static bool SyslogClientSendLocalFormat(SyslogClient* self, int severity,
                                        double unixTime, const char* message,
                                        size_t messageSize) {
  time_t time = lround(floor(unixTime));
  struct tm tm;
  localtime_r(&time, &tm);
  char header[32];
  size_t headerSize = 0;
  headerSize += snprintf(header + headerSize, sizeof(header) - headerSize,
                         "<%d>", self->facility | severity);
  headerSize += strftime(header + headerSize, sizeof(header) - headerSize,
                         "%h %e %T ", &tm);
  char pid[32];
  size_t pid_size = snprintf(pid, sizeof(pid), "[%lu]: ", (long)getpid());
  size_t iovcnt = 0;
  struct iovec iov[4];
  iov[iovcnt].iov_base = header;
  iov[iovcnt].iov_len = headerSize;
  ++iovcnt;
  iov[iovcnt].iov_base = self->tag;
  iov[iovcnt].iov_len = self->tagSize;
  ++iovcnt;
  iov[iovcnt].iov_base = pid;
  iov[iovcnt].iov_len = pid_size;
  ++iovcnt;
  iov[iovcnt].iov_base = (void*)message;
  iov[iovcnt].iov_len = messageSize;
  ++iovcnt;
  assert(iovcnt <= sizeof(iov) / sizeof(iov[0]));
  return self->transport->send(self->transport, iov, iovcnt);
}

static bool SyslogClientSendRemoteFormat(SyslogClient* self, int severity,
                                         double unixTime, const char* message,
                                         size_t messageSize) {
  time_t time = lround(floor(unixTime));
  struct tm tm;
  localtime_r(&time, &tm);
  char header[32];
  size_t headerSize = 0;
  headerSize += snprintf(header + headerSize, sizeof(header) - headerSize,
                         "<%d>", self->facility | severity);
  headerSize += strftime(header + headerSize, sizeof(header) - headerSize,
                         "%h %e %T ", &tm);
  char pid[32];
  size_t pidSize = snprintf(pid, sizeof(pid), "[%lu]: ", (long)getpid());
  size_t iovcnt = 0;
  struct iovec iov[6];
  iov[iovcnt].iov_base = header;
  iov[iovcnt].iov_len = headerSize;
  ++iovcnt;
  iov[iovcnt].iov_base = self->hostname;
  iov[iovcnt].iov_len = self->hostnameSize;
  ++iovcnt;
  iov[iovcnt].iov_base = " ";
  iov[iovcnt].iov_len = 1;
  ++iovcnt;
  iov[iovcnt].iov_base = self->tag;
  iov[iovcnt].iov_len = self->tagSize;
  ++iovcnt;
  iov[iovcnt].iov_base = pid;
  iov[iovcnt].iov_len = pidSize;
  ++iovcnt;
  iov[iovcnt].iov_base = (void*)message;
  iov[iovcnt].iov_len = messageSize;
  ++iovcnt;
  assert(iovcnt <= sizeof(iov) / sizeof(iov[0]));
  return self->transport->send(self->transport, iov, iovcnt);
}

bool SyslogClientSend(SyslogClient* self, int severity, double unixTime,
                      const char* message, size_t message_size) {
  if (self && self->transport) {
    if (self->messageFormat == SYSLOG_MESSAGE_FORMAT_LOCAL) {
      return SyslogClientSendLocalFormat(self, severity, unixTime, message,
                                         message_size);
    } else if (self->messageFormat == SYSLOG_MESSAGE_FORMAT_REMOTE) {
      return SyslogClientSendRemoteFormat(self, severity, unixTime, message,
                                          message_size);
    }
  }
  return false;
}
