#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct iovec;

typedef struct SyslogTransport SyslogTransport;

typedef bool (*SyslogTransportSendFunc)(SyslogTransport*, const struct iovec*,
                                        size_t);
typedef void (*SyslogTransportDestroyFunc)(SyslogTransport*);

/**
 * Base class for a transport.
 */
struct SyslogTransport {
  /**
   * (virtual method) Sends a message without bufferization.
   * Returns true if successful.
   */
  SyslogTransportSendFunc send;

  /**
   * (virtual method) Destructor.
   */
  SyslogTransportDestroyFunc destroy;
};

/**
 * Create a default transport to local syslog deamon.
 */
SyslogTransport* SyslogTransportCreateDefault();

/**
 * Destroy syslog transport.
 */
static inline void SyslogTransportDestroy(SyslogTransport* self) {
  if (self) {
    (*self->destroy)(self);
  }
}

#ifdef __cplusplus
}  // extern "C"
#endif
