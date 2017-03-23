#pragma once
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

typedef struct SyslogClient SyslogClient;

/**
 * Creates a syslog client for local syslog deamon.
 */
SyslogClient* SyslogClientCreateDefault(int facility, const char* tag);

/**
 * Creates a syslog client with a specific message format and based on a
 * specific transport. Ownership of the transport is transferred to the client.
 */
SyslogClient* SyslogClientCreate(SyslogMessageFormat messageFormat,
                                 SyslogTransport* transport, int facility,
                                 const char* tag);

/**
 * Destroy syslog client.
 */
void SyslogClientDestroy(SyslogClient* self);

/**
 * Send a formatted message to syslog.
 */
bool SyslogClientPrintf(SyslogClient* self, int severity, const char* format,
                        ...);

/**
 * Send a message to syslog.
 */
bool SyslogClientSend(SyslogClient* self, int severity, struct timeval* tv,
                      const char* message, size_t messageSize);

#ifdef __cplusplus
}  // extern "C"
#endif
