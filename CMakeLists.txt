cmake_minimum_required(VERSION 2.8.4)
project(syslog_client)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

add_library(syslog_client
    syslog_client.c
    syslog_client.h
    syslog_facility.h
    syslog_message_format.h
    syslog_severity.h
    syslog_transport.c
    syslog_transport.h
    unix_socket.c
    unix_socket.h)

add_executable(sample sample.c)
target_link_libraries(sample syslog_client)
