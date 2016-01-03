.PHONY: all clean

SRCS = \
	syslog_client.c \
	syslog_local_transport.c \
	syslog_local_transport.h \
	unix_socket.c \
	unix_socket.h \
	include/syslog_client.h \
	include/syslog_client_ext.h

CPPFLAGS += -Wall -Wextra -pedantic -std=c99 -I include
OPTFLAGS += -g -O2

all: libsyslog_client.so sample

libsyslog_client.so: ${SRCS}
	${CC} ${CPPFLAGS} ${OPTFLAGS} -shared -o $@ $(filter %.c, ${SRCS})

sample: libsyslog_client.so sample.o
	${CC} ${CPPFLAGS} ${OPTFLAGS} -o $@ sample.c -L . -lsyslog_client

clean:
	${RM} libsyslog_client.so sample
