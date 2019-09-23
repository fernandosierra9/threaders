CC = gcc
RM = rm -f

TARGET = sac_server.exe
SRC = $(wildcard *.c) $(wildcard */*.c) $(wildcard */*/*.c)
SRC += ../shared-common/common/config.c ../shared-common/common/logger.c
OBJ = $(SRC:.c=.o)
HEADER = $(SRC:.c=.h) defines.h
LIB = -lcommons -lpthread -lfuse

%.o: %.c $(HEADER)
	$(CC) $(CCFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CCFLAGS) $(OBJ) -o $@ $(LIB)

debug: CC += -DDEBUG -gF

debug: all

all: $(TARGET)

clean:
	$(RM) $(OBJ)
	$(RM) $(TARGET)

.PHONY: all clean