EXEC=test
CC=gcc
CFLAGS=-Wall -g

SRCS=$(shell find ./ -name *.c)

OBJS := $(SRCS:%=%.o)
DEPS := $(OBJS:.o=.d)

$(EXEC): $(OBJS)
		$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.c.o: %.c
		$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:	
	rm -r test.c.o test 

-include $(DEPS)
