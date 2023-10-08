#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h> // for open

#include "commands.h"

void execute_command(char* command, int input_fd, int output_fd) {
    //redirect input if 
    if (input_fd != STDIN_FILENO) {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }

    //Determine whether to append or overwrite output
    int output_flags = O_WRONLY | O_CREAT;
    if (output_fd != STDOUT_FILENO) {
        if (output_fd == STDERR_FILENO){
            //Redirecting to STDERR
            dup2(STDERR_FILENO, STDOUT_FILENO);
            close(STDERR_FILENO);
        }
        else if (output_fd == STDOUT_FILENO + 1){
            //Append(>>) to the output file
            output_flags |= O_APPEND;
            output_fd = STDOUT_FILENO; //Reset output_fd to STDOUT_FILENO
        }
        else{
            //Redirect to the specified file
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }
    }

    // printf("Input fd: %d, output fd: %d\n", input_fd, output_fd);

    char* arguments[50];
    char* arg = strtok(command, " ");
    int arg_counter = 0;
    int input_redirection = 0;
    int output_redirection = 0;
    char input_file[50];
    char output_file[50];

    while (arg != NULL) {
        if (strcmp(arg, "<") == 0){
            arg = strtok(NULL, " "); //Get the input file name
            if (arg != NULL) {
                strcpy(input_file, arg);
                input_redirection = 1;
            }
        }
        else if (strcmp(arg, ">") == 0){
            arg = strtok(NULL, " "); //Get the output file name
            if (arg != NULL) {
                strcpy(output_file, arg);
                output_redirection = 1;
            }
        }
        else if (strcmp(arg, ">>") == 0){
            arg = strtok(NULL, " "); //Get the output file name
            if (arg != NULL) {
                strcpy(output_file, arg);
                output_redirection = 2;
            }
        }
        else{
            arguments[arg_counter] = strdup(arg);    
            arg_counter++;
        }
        arg = strtok(NULL, " ");
    }
    arguments[arg_counter] = NULL;

    //Checking for missing argunments
    // if (arguments[0] == NULL) {
    //     fprintf(stderr, "Command missing arguments.\n");
    //     exit(1);
    // }

    // Input redirection
    if (input_redirection) {
        int input_fd = open(input_file, O_RDONLY);
        if (input_fd == -1) {
            perror("Input redirection error");
            exit(1);
        }
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }

    // Output redirection
    if (output_redirection) {
        int output_fd;
        if (output_redirection == 1) {
            // Redirect with truncation (`>`)
            output_fd = open(output_file, output_flags | O_TRUNC, 0666);
        } else {
            // Redirect with append (`>>`)
            output_fd = open(output_file, output_flags | O_APPEND, 0666);
        }
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
    }
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