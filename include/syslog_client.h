#ifndef SYSLOG_CLIENT_H_
#define SYSLOG_CLIENT_H_ 1

#include "syslog_facility.h"
#include "syslog_message_format.h"
#include "syslog_severity.h"
#include "syslog_transport.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct timeval;

typedef struct syslog_client syslog_client;

/**
 * Creates a syslog client for local syslog deamon.
 */
syslog_client *syslog_client_create_default(int facility, const char *tag);

/**
 * Creates a syslog client with a specific message format and based on a
 * specific transport. Ownership of the transport is transferred to the client.
 */
syslog_client *syslog_client_create(syslog_message_format message_format,
                                    syslog_transport *transport, int facility,
                                    const char *tag);

/**
 * Destroy syslog client.
 */
void syslog_client_destroy(syslog_client *self);

/**
 * Send a formatted message to syslog.
 */
bool syslog_client_printf(syslog_client *self, int serverity,
                          const char *format, ...);

/**
 * Send a message to syslog.
 */
bool syslog_client_send(syslog_client *self, int serverity, struct timeval *tv,
                        const char *message, size_t message_size);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // SYSLOG_CLIENT_H_
