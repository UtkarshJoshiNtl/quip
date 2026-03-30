CC = gcc
CFLAGS = -Wall -Wextra -O2
SRC = src/main.c
OBJ = main.o
TARGET = quip

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
