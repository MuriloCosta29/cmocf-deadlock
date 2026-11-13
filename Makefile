CC = gcc
CFLAGS = -Wall -Wextra -std=c99

OBJS = main.o banker.o parser.o

banker: $(OBJS)
	$(CC) $(CFLAGS) -o banker $(OBJS)

%.o: %.c banker.h parser.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) banker

.PHONY: clean
