CC = gcc
CFLAGS = -Wall -g
TARGET = zpm

all: $(TARGET)

$(TARGET): src/zpm.o
	$(CC) $(CFLAGS) -o $@ $^

src/zpm.o: src/zpm.c src/zpm.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) src/*.o