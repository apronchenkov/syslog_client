#ifndef SYSLOG_TRANSPORT_H_
#define SYSLOG_TRANSPORT_H_ 1

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct iovec;

typedef struct syslog_transport syslog_transport;

typedef bool (*syslog_transport_send_func)(syslog_transport*,
                                           const struct iovec*, size_t);
typedef void (*syslog_transport_destroy_func)(syslog_transport*);

/**
 * Base class for a transport.
 */
struct syslog_transport {
  /**
   * (virtual method) Sends a message without bufferization.
   * Returns true if successful.
   */
  syslog_transport_send_func send;

  /**
   * (virtual method) Destructor.
   */
  syslog_transport_destroy_func destroy;
};

/**
 * Create a default transport to local syslog deamon.
 */
syslog_transport* syslog_transport_create_default();

/**
 * Destroy syslog transport.
 */
static inline void syslog_transport_destroy(syslog_transport* self) {
  if (self) {
    (*self->destroy)(self);
  }
}

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // SYSLOG_TRANSPORT_H_
