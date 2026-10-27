CC = gcc
CFLAGS = -Wall -Wextra -Werror -O2 -std=c17
SRC = src/main.c src/terminal.c src/prompt.c \
      src/builtins.c src/execute.c src/signals.c
TARGET = quip

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

clean:
	rm -f $(TARGET) *.o

test: $(TARGET)
	@echo "No tests yet"

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

.PHONY: all clean debug test install
