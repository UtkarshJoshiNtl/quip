CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
SRC = src/main.c src/history.c src/terminal.c src/prompt.c src/builtins.c src/execute.c
OBJ = main.o history.o terminal.o prompt.o builtins.o execute.o
TARGET = quip

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJ)

test: $(TARGET)
	./quip -c "help"

.PHONY: all clean debug test
