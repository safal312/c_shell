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
    
        if (strcmp(tokens[0], "exit") == 0) {
            for (int i = 0; i < token_count; i++) {
                free(tokens[i]); // Free the memory allocated for each token
            }
            exit(0);
        }

        pid_t pid = fork();

        if (pid < 0){
            perror("fork failed");
            exit(1);
        }
        else if(pid == 0){
            //child process
            execute_command(tokens);
            printf("Hi %d\n", pid);
        }
        else{
            //parent process
            int status;
            waitpid(pid, &status, 0);
        }
        printf("Helloooo %d\n", pid);
        for (int i = 0; i < token_count; i++) {
            printf("Token %d: %s\n", i, tokens[i]);
            free(tokens[i]); // Free the memory allocated for each token
        }
    }

    return 0;
}