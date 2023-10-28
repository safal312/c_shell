a.out: main.c utils/parser.c shell_commands/commands.c
	gcc -Wall -o $@ $^

c.out: client.c
	gcc -Wall -o $@ $^

all: a.out c.out

clean:
	rm -f a.out

.PHONY: clean