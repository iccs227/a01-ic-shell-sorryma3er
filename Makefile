CC=gcc
CFLAGS=-Wall -g -Iinclude # go to /include to find header files
BINARY=icsh

#define src, include, obj, bin directory
SRCDIR = src
INDIR = include
OBJDIR = obj
BINDIR = . # current dir

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

all: $(BINDIR)/$(BINARY)

# compile
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# link
$(BINDIR)/$(BINARY): $(OBJS)
	$(CC) -o $(BINARY) $(CFLAGS) $^

.PHONY: clean
clean:
	rm -f $(BINARY) $(OBJS)
