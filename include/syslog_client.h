#ifndef SYSLOG_CLIENT_H_
#define SYSLOG_CLIENT_H_ 1

#include <stdbool.h>

#define SYSLOG_SEVERITY_EMERGENCY (0)
#define SYSLOG_SEVERITY_ALERT (1)
#define SYSLOG_SEVERITY_CRITICAL (2)
#define SYSLOG_SEVERITY_ERROR (3)
#define SYSLOG_SEVERITY_WARNING (4)
#define SYSLOG_SEVERITY_NOTICE (5)
#define SYSLOG_SEVERITY_INFO (6)
#define SYSLOG_SEVERITY_DEBUG (7)

#define SYSLOG_FACILITY_KERN (0 << 3)
#define SYSLOG_FACILITY_USER (1 << 3)
#define SYSLOG_FACILITY_MAIL (2 << 3)
#define SYSLOG_FACILITY_DAEMON (3 << 3)
#define SYSLOG_FACILITY_AUTH (4 << 3)
#define SYSLOG_FACILITY_SYSLOG (5 << 3)
#define SYSLOG_FACILITY_LPR (6 << 3)
#define SYSLOG_FACILITY_NEWS (7 << 3)
#define SYSLOG_FACILITY_UUCP (8 << 3)
#define SYSLOG_FACILITY_CRON (9 << 3)
#define SYSLOG_FACILITY_AUTHPRIV (10 << 3)
#define SYSLOG_FACILITY_FTP (11 << 3)
#define SYSLOG_FACILITY_LOCAL0 (16 << 3)
#define SYSLOG_FACILITY_LOCAL1 (17 << 3)
#define SYSLOG_FACILITY_LOCAL2 (18 << 3)
#define SYSLOG_FACILITY_LOCAL3 (19 << 3)
#define SYSLOG_FACILITY_LOCAL4 (20 << 3)
#define SYSLOG_FACILITY_LOCAL5 (21 << 3)
#define SYSLOG_FACILITY_LOCAL6 (22 << 3)
#define SYSLOG_FACILITY_LOCAL7 (23 << 3)

struct timeval;

typedef struct syslog_client syslog_client;

syslog_client *syslog_open(int facility, const char *tag);

bool syslog_send(syslog_client *self, int serverity, const char *format, ...);

void syslog_close(syslog_client *self);

#endif /* SYSLOG_CLIENT_H_ */
