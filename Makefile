# CC=x86_64-w64-mingw32-gcc
CC=gcc
CFLAGS=-I'include/' -Wuninitialized -Wall -Wextra -pedantic -std=c99 -g


INCLUDE=include
DEPS=nanoc util types map error
SRC=src
OBJ=obj
OBJS=error util map lexer parser interpreter

_DEPS=$(patsubst %, $(INCLUDE)/%.h, $(DEPS))
_OBJS=$(patsubst %, $(OBJ)/%.o, $(OBJS))

TEST_SRC=test/src
TEST_OBJ=test/obj
TEST_DEPS=readfile

TARGET=nanoc

TEST_OBJS=main readfile
_TEST_OBJS=$(patsubst %, $(TEST_OBJ)/%.o, $(TEST_OBJS))

$(OBJ)/%.o: $(SRC)/%.c $(_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TEST_OBJ)/%.o: $(TEST_SRC)/%.c $(_TEST_DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -I'test/include/'

nanoc: $(_OBJS) $(_TEST_OBJS)
	$(CC) -o $(TARGET) $(_OBJS) $(_TEST_OBJS)

.PHONY: clean
clean:
	rm -f $(OBJ)/*.o
	rm -f $(TEST_OBJ)/*.o
