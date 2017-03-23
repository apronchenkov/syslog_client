cmake_minimum_required(VERSION 2.8.4)
project(syslog_client)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

add_library(syslog_client
    SyslogClient.c
    SyslogClient.h
    SyslogFacility.h
    SyslogMessageFormat.h
    SyslogSeverity.h
    SyslogTransport.c
    SyslogTransport.h
    UnixSocket.c
    UnixSocket.h)

add_executable(sample sample.c)
target_link_libraries(sample syslog_client)
