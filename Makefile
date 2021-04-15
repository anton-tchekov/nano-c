CC=gcc
CFLAGS=-I. -Wuninitialized -Wall -Wextra -pedantic -std=c99 -g
DEPS=main.h types.h map.h
TARGET=ci
OBJS = \
	obj/map.o \
	obj/readfile.o \
	obj/lexer.o \
	obj/parser.o \
	obj/interpreter.o \
	obj/main.o

obj/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -fstack-usage

ci: $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

.PHONY: clean
clean:
	rm -f obj/*.o
