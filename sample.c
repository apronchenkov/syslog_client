#include "syslog_client.h"

int main(int argc, char** argv) {
  (void)argc;
  syslog_client* const syslog =
      syslog_client_create_default(SYSLOG_FACILITY_USER, argv[0]);
  syslog_client_printf(syslog, SYSLOG_SEVERITY_DEBUG, "Debug message.");
  syslog_client_printf(syslog, SYSLOG_SEVERITY_INFO, "Info message.");
  syslog_client_printf(syslog, SYSLOG_SEVERITY_NOTICE, "Notice message.");
  syslog_client_printf(syslog, SYSLOG_SEVERITY_WARNING, "Warning message.");
  syslog_client_printf(syslog, SYSLOG_SEVERITY_ERROR, "Error message.");
  syslog_client_printf(syslog, SYSLOG_SEVERITY_CRITICAL, "Critical message.");
  syslog_client_printf(syslog, SYSLOG_SEVERITY_ALERT, "Alert message.");
  syslog_client_printf(syslog, SYSLOG_SEVERITY_EMERGENCY, "Emergency message.");
  return 0;
}
