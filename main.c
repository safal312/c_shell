#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "utils/parser.h"
#include "shell_commands/commands.h"

int main() {
    char input[MAX_INPUT];
    char* tokens[MAX_TOKENS];

    while (1){
        printf("terminal> ");
        fgets(input, sizeof(input), stdin);
        
        int token_count = parse_input(input, tokens);

        // check if input is empty or made with only whitespace characters
        if (token_count == 0) {
            continue;
        }

        if (strcmp(tokens[0], "exit") == 0) {
            for (int i = 0; i < token_count; i++) {
                free(tokens[i]); // Free the memory allocated for each token
            }
            exit(0);
        }

        execute(tokens, token_count);
    }

    return 0;
}