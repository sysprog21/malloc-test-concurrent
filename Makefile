EXEC = malloc-test-concurrent
.PHONY: all
all: $(EXEC)

CC ?= gcc
CFLAGS = -std=gnu99 -Wall -O2 -g
LDFLAGS = -lpthread

OBJS := \
	malloc-test-concurrent.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

ARGS = --thread=4 --size=1 --count=10 --loop=1000
check: $(EXEC)
	$(shell dirname $(EXEC))/$(EXEC) $(ARGS)

clean:
	$(RM) $(EXEC) $(OBJS)
