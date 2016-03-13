.PHONY: all clean

SRCS = \
	include/syslog_client.h \
	include/syslog_facility.h \
	include/syslog_message_format.h \
	include/syslog_severity.h \
	include/syslog_transport.h \
	syslog_client.c \
	syslog_transport.c \
	unix_socket.c \
	unix_socket.h

CPPFLAGS += -Wall -Wextra -pedantic -std=c99 -I include
OPTFLAGS += -g -O2

all: libsyslog_client.so sample

libsyslog_client.so: ${SRCS}
	${CC} ${CPPFLAGS} ${OPTFLAGS} -fPIC -shared -o $@ $(filter %.c, ${SRCS})

sample: libsyslog_client.so sample.o
	${CC} ${CPPFLAGS} ${OPTFLAGS} -o $@ sample.c -L . -lsyslog_client

clean:
	${RM} libsyslog_client.so sample
