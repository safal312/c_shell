#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "commands.h"

void execute_command(char* command, int input_fd, int output_fd) {
    //redirect input
    if (input_fd != STDIN_FILENO) {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }

    //redirect output
    if (output_fd != STDOUT_FILENO) {
        dup2(output_fd, STDOUT_FILENO);
        // close(output_fd);
    }

    // printf("Input fd: %d, output fd: %d\n", input_fd, output_fd);

    char* arguments[50];
    char* arg = strtok(command, " ");
    int arg_counter = 0;

    while (arg != NULL) {
        arguments[arg_counter] = strdup(arg);    
        arg = strtok(NULL, " ");
        arg_counter++;
    }
    arguments[arg_counter] = NULL;

    int status = execvp(arguments[0], arguments);

    //exit if failed to run command
    if (status == -1) {
        perror("Error executing command");
        exit(1);
    }
}

void execute(char** commands, char* delims, int commands_count) {
    int pipes[commands_count-1][2];

    for (int i = 0; i < commands_count; i++) {
        if (i != commands_count - 1 && delims[i] == '|' && pipe(pipes[i]) == -1) {
            perror("pipe failed");
            exit(1);
        }
        pid_t pid = fork();

        if (pid < 0){
            perror("fork failed");
            exit(1);
        }
        else if(pid == 0){
            //child process
            int input_fd = STDIN_FILENO;
            int output_fd = STDOUT_FILENO;

            // if the command is not the first one, redirect input to the reading end of pipe
            if (i != 0) {
                input_fd = pipes[i-1][0];
            } else {
                // if it is the first command, close the reading end of pipe
                close(pipes[i][0]);
            }

            // if the command is not the last one, redirect output to the writing end of pipe
            if (i != commands_count-1) {
                output_fd = pipes[i][1];
            } 

            execute_command(commands[i], input_fd, output_fd);
        }
        else{
            // we need to close writing end from parent so that it doesn't block the program
            // if it wasn't closed child would wait for the parent to write something
            if (i != commands_count - 1) close(pipes[i][1]);
            // starting from the second command, we need to close reading end from parent
            // because we won't use it anymore
            if (i != 0) close(pipes[i-1][0]);
            //parent process
            int status;
            waitpid(pid, &status, 0);
            
            
        }
        // execute_command(commands[i], STDIN_FILENO, STDOUT_FILENO);
    }
}