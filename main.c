#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "utils/parser.h"
#include "shell_commands/commands.h"

int main() {
    char input[MAX_INPUT];
    char* tokens[MAX_TOKENS];

    printf("terminal> ");
    fgets(input, sizeof(input), stdin);

    int token_count = parse_input(input, tokens);
    tokens[token_count] = NULL;
    execute_command(tokens);
    
    for (int i = 0; i < token_count; i++) {
        printf("Token %d: %s\n", i, tokens[i]);
        free(tokens[i]); // Free the memory allocated for each token
    }

    return 0;
}