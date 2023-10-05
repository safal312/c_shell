#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "commands.h"

void execute_command(char** arguments) {
    int status = execvp(arguments[0], arguments);

    //exit if failed to run command
    if (status == -1) {
        perror("Error executing command");
        exit(1);
    }
}

void execute(char** tokens, int token_count) {
    pid_t pid = fork();

    if (pid < 0){
        perror("fork failed");
        exit(1);
    }
    else if(pid == 0){
        //child process
        execute_command(tokens);
    }
    else{
        //parent process
        int status;
        waitpid(pid, &status, 0);
    }

    for (int i = 0; i < token_count; i++) {
        free(tokens[i]); // Free the memory allocated for each token
    }
}