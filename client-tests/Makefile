CC = gcc
RM = rm -f

TARGET = client-tests.exe
SRC = $(wildcard *.c) $(wildcard */*.c) ../hilolay/hilolay.c ../libmuse/libmuse.c
OBJ = $(SRC:.c=.o) 
HEADER = $(SRC:.c=.h) defines.h


%.o: %.c $(HEADER)
	$(CC) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

debug: CC += -DDEBUG -g

debug: all

all: $(TARGET)

clean:
	$(RM) $(OBJ)
	$(RM) $(TARGET)

.PHONY: all clean