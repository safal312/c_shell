#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "utils/parser.h"
#include "shell_commands/commands.h"

int main() {
    char input[MAX_INPUT];          // array to store input from the terminal
    char* commands[MAX_COMMANDS];   // array to store the commands after parsing input
    char delims[MAX_COMMANDS-1];    // array to store the delimiters after parsing input

    while (1){
        printf("terminal> ");
        fgets(input, sizeof(input), stdin);
        
        int commands_count = parse_input(input, commands, delims);

        // check if input is empty or made with only whitespace characters
        if (commands_count == 0) {
            continue;
        }

        if (strcmp(commands[0], "exit") == 0) {
            for (int i = 0; i < commands_count; i++) {
                free(commands[i]); // Free the memory allocated for each token
            }
            exit(0);
        }

        execute(commands, delims, commands_count);
    }

    return 0;
}