cmake_minimum_required(VERSION 2.8.4)
project(syslog_client)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

add_library(syslog_client
    SyslogClient.c
    SyslogTransport.c
    UnixSocket.c
    UnixSocket.h
    public/SyslogClient.h
    public/SyslogFacility.h
    public/SyslogMessageFormat.h
    public/SyslogSeverity.h
    public/SyslogTransport.h
)

add_executable(sample sample.c)
target_link_libraries(sample syslog_client)
