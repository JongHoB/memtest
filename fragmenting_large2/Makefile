EXEC=test_0 test_1
CC=gcc
CFLAGS=-Wall -g

# 명시적으로 소스 파일들을 지정
SRCS=test_0.c test_1.c

OBJS := $(SRCS:.c=.o)
DEPS := $(OBJS:.o=.d)

all: $(EXEC)

test_0: test_0.o
	$(CC) $< -o $@ $(LDFLAGS)

test_1: test_1.o
	$(CC) $< -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) $(EXEC)

-include $(DEPS)

