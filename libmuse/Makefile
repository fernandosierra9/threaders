
CC = gcc
CFLAGS = -fPIC -Wall -Wextra -O2 -c
LDFLAGS = -shared
RM = rm -f
TARGET_LIB = libmuse.so # target lib
LIB = -lcommons

SRCS = libmuse.c ../shared-common/common/sockets.c
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^ $(LIB)

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)