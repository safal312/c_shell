# a.out: main.c utils/parser.c shell_commands/commands.c
# 	gcc -Wall -o $@ $^

# c.out: client.c
# 	gcc -Wall -o $@ $^

# all: a.out c.out

# clean:
# 	rm -f a.out

# .PHONY: clean

CC = gcc
CFLAGS = -Wall -pthread

server: main.c utils/parser.c shell_commands/commands.c
	$(CC) $(CFLAGS) -o $@ $^

client: client.c
	$(CC) $(CFLAGS) -o $@ $^

all: server client

clean:
	rm -f server client

.PHONY: clean