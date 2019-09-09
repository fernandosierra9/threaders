CC = gcc
RM = rm -f

TARGET = memoria
SRC = $(wildcard *.c) $(wildcard */*.c)
OBJ = $(SRC:.c=.o)
HEADER = $(SRC:.c=.h) defines.h
LIB = -lcommons

%.o: %.c $(HEADER)
	$(CC) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIB)

debug: CC += -DDEBUG -g

debug: all

all: $(TARGET)

clean:
	$(RM) $(OBJ)
	$(RM) $(TARGET)

.PHONY: all clean