#ifndef SYSLOG_CLIENT_EXT_H_
#define SYSLOG_CLIENT_EXT_H_ 1

#include "syslog_client.h"
#include <stddef.h>

struct iovec;

typedef struct syslog_transport syslog_transport;

typedef bool (*syslog_transport_send_func)(syslog_transport *,
                                           const struct iovec *, size_t);
typedef void (*syslog_transport_destroy_func)(syslog_transport *);

struct syslog_transport {
  syslog_transport_send_func send;
  syslog_transport_destroy_func destroy;
};

enum syslog_message_format {
  SYSLOG_MESSAGE_FORMAT_LOCAL,
  SYSLOG_MESSAGE_FORMAT_REMOTE
};

typedef enum syslog_message_format syslog_message_format;

syslog_client *syslog_open_ext(syslog_message_format message_format,
                               syslog_transport *transport, int facility,
                               const char *tag);

bool syslog_send_ext(syslog_client *self, int serverity, struct timeval *tv,
                     const char *message, size_t message_size);

#endif /* SYSLOG_CLIENT_EXT_H_ */
