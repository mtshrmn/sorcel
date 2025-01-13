CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -Oz
INCLUDE := -lm
SRCDIR := src
SRC := $(wildcard $(SRCDIR)/*.c)
EXECUTABLE := sorcel

build: $(SRC)
	$(CC) $(CFLAGS) $(INCLUDE) $(SRC) -o $(EXECUTABLE)

