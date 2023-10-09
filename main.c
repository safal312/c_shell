#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "utils/parser.h"           // for parse_input
#include "shell_commands/commands.h"        // for executing commands

int main() {
    char input[MAX_INPUT];          // array to store input from the terminal
    char* commands[MAX_COMMANDS];   // array to store the commands after parsing input
    
    // infinite loop for the terminal
    while (1){
        printf("terminal> ");
        fgets(input, sizeof(input), stdin);         // get input from the terminal
        
        // parse the input and store the commands in the commands array
        // this separates the commands by pipes
        int commands_count = parse_input(input, commands);

        // check if input is empty or made with only whitespace characters
        if (commands_count == 0) {
            continue;
        }

        // check if the user wants to exit the terminal
        if (strcmp(commands[0], "exit") == 0) {
            exit(0);
        }
        
        // execute the commands
        execute(commands, commands_count);
    }

    return 0;
}