a.out: main.c utils/parser.c shell_commands/commands.c
	gcc -Wall -o $@ $^

clean:
	rm -f a.out

.PHONY: clean