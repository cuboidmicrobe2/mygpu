CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g -I../../include

LIBRARY = ../../build/libmygpu.a
TARGET = ../../build/examples/triangle

SOURCE = main.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCE) $(LIBRARY)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(SOURCE) $(LIBRARY) -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)