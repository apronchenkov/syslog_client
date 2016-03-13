#ifndef SYSLOG_MESSAGE_FORMAT_H_
#define SYSLOG_MESSAGE_FORMAT_H_ 1

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum syslog_message_format {
  SYSLOG_MESSAGE_FORMAT_LOCAL,
  SYSLOG_MESSAGE_FORMAT_REMOTE
};

typedef enum syslog_message_format syslog_message_format;

#ifdef __cplusplus
} // extern "C"
#endif
#endif // SYSLOG_MESSAGE_FORMAT_H_
