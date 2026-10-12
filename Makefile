CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
SRC = src/main.c src/huffman.c src/heap.c src/bitio.c
OBJ = $(SRC:.c=.o)
TARGET = huffman

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
