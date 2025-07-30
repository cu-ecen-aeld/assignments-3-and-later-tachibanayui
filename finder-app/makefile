CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -Werror
.PHONY: all
all: writer

writer: writer.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f writer