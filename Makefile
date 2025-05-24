CC=gcc
CFLAGS=-Wall -g -Iinclude # go to /include to find header files
BINARY=icsh

#define src, include, obj, bin directory
SRCDIR = src
INDIR = include
OBJDIR = obj

SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

all: $(BINARY)

# ensure the object directory exists
$(OBJDIR):
	mkdir -p $(OBJDIR)

# compile
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR) # prerequisite for obj directory exists only
	$(CC) $(CFLAGS) -c $< -o $@

# link
$(BINARY): $(OBJS)
	$(CC) -o $(BINARY) $(CFLAGS) $^

.PHONY: clean
clean:
	rm -f $(BINARY) $(OBJS)
