EXEC = malloc-test-concurrent

GIT_HOOKS := .git/hooks/pre-commit
.PHONY: all
all: $(GIT_HOOKS) $(EXEC)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

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
