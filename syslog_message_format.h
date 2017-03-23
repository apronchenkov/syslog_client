#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum SyslogMessageFormat {
  SYSLOG_MESSAGE_FORMAT_LOCAL,
  SYSLOG_MESSAGE_FORMAT_REMOTE
};

typedef enum SyslogMessageFormat SyslogMessageFormat;

#ifdef __cplusplus
}  // extern "C"
#endif
