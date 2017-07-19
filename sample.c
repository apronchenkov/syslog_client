#include <github.com/apronchenkov/syslog_client/public/SyslogClient.h>

int main(int argc, char **argv) {
  (void)argc;
  SyslogClient *const syslog =
      SyslogClientCreateDefault(SYSLOG_FACILITY_USER, argv[0]);
  SyslogClientPrintf(syslog, SYSLOG_SEVERITY_DEBUG, "Debug message.");
  SyslogClientPrintf(syslog, SYSLOG_SEVERITY_INFO, "Info message.");
  SyslogClientPrintf(syslog, SYSLOG_SEVERITY_NOTICE, "Notice message.");
  SyslogClientPrintf(syslog, SYSLOG_SEVERITY_WARNING, "Warning message.");
  SyslogClientPrintf(syslog, SYSLOG_SEVERITY_ERROR, "Error message.");
  SyslogClientPrintf(syslog, SYSLOG_SEVERITY_CRITICAL, "Critical message.");
  SyslogClientPrintf(syslog, SYSLOG_SEVERITY_ALERT, "Alert message.");
  SyslogClientPrintf(syslog, SYSLOG_SEVERITY_EMERGENCY, "Emergency message.");
  return 0;
}
