.PHONY: all clean

SRCS = \
	syslog_client.h \
	syslog_facility.h \
	syslog_message_format.h \
	syslog_severity.h \
	syslog_transport.h \
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
