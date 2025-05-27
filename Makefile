CC=gcc
CFLAGS=-Wall -g 
BINARY=icsh
SRCS=icsh.c exec.c execscript.c preparecmd.c sigHandler.c util.c background.c
OBJS=$(SRCS:.c=.o)

all: $(BINARY)

icsh: $(SRCS) 
	$(CC) $(CFLAGS) -o $(BINARY) $(SRCS)

.PHONY: clean

clean:
	rm -f $(BINARY)
