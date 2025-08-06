CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -g -Werror
.PHONY: all
all: aesdsocket

aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f writer