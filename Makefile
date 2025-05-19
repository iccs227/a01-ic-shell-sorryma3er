CC=gcc
CFLAGS=-Wall -g 
BINARY=icsh

SRCS=icsh.c command.c builtin.c script.c exec.c signal.c redirect.c
OBJS=$(SRCS:.c=.o)

all: icsh

icsh: $(OBJS)
	$(CC) -o $(BINARY) $(CFLAGS) $^


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(BINARY) $(OBJS)
