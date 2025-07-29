CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -Werror


writer: writer.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f writer