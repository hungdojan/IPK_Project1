CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g -pedantic
CFLAGS+=-DDEBUG
TARGET=hinfosvc
OBJS= main.o responses.o

.PHONY: all clean run pack

all: $(TARGET)

run: $(TARGET)
	./$< 12345

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGET) *.zip

pack:
	rm -f xdohun00.zip
	zip xdohun00 Makefile *.c *.h README.md

