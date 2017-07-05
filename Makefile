PROJECT = github.com/apronchenkov/syslog_client
BIN_PATH = ${TG_PATH}/bin
PKG_PATH = ${TG_PATH}/pkg/${PROJECT}
SRC_PATH = ${TG_PATH}/src/${PROJECT}

.PHONY: all clean

SRCS = \
	SyslogClient.c \
	SyslogTransport.c \
	UnixSocket.c \
	UnixSocket.h \
	public/SyslogClient.h \
	public/SyslogFacility.h \
	public/SyslogMessageFormat.h \
	public/SyslogSeverity.h \
	public/SyslogTransport.h \

CPPFLAGS += -Wall -Wextra -pedantic -isystem ${TG_PATH}/src
CFLAGS += -std=c99
OPTFLAGS += -g -O2

all: ${PKG_PATH}/syslog_client.a ${BIN_PATH}/sample

${PKG_PATH}/%.o: ${SRC_PATH}/%.c
	@mkdir -p $(dir $@)
	${CC} ${CFLAGS} ${CPPFLAGS} ${OPTFLAGS} -c -o $@ $^

${PKG_PATH}/syslog_client.a: $(addprefix ${PKG_PATH}/, $(patsubst %.c, %.o, $(filter %.c, ${SRCS})))
	ar rcs $@ $^

${BIN_PATH}/sample: sample.o ${PKG_PATH}/syslog_client.a
	${CC} ${LDFLAGS} ${OPTFLAGS} -o $@ $^

clean:
	${RM} ${BIN_PATH}/sample ${PKG_PATH}/*.a ${PKG_PATH}/*.o
