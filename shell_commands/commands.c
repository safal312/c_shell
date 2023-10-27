#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h> // for open
#include <sys/socket.h>

#include "commands.h"
#include "../utils/parser.h"

void execute_command(char* command, int input_fd, int output_fd) {
    //redirect input if input_fd is not standard input
    if (input_fd != STDIN_FILENO) {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }

    // redirect output if output_fd is not standard output
    if (output_fd != STDOUT_FILENO) {
        //Redirect to the specified file
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
    }

    char* arguments[50];            // array to store each token of a command
    int arg_counter = 0;            // count tokens in command
    int input_redirection = 0;      // binary flag to check if input redirection is needed
    int output_redirection = 0;     // binary flag to check if output redirection is needed
    int error_redirection = 0;      // binary flag to check if error redirection is needed
    // filenames for input and output redirection
    char input_file[50];
    char output_file[50];
    char error_file[50];

    // iterate over all tokens in command separated by space
    char* arg = strtok(command, " ");
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
        } else if (strcmp(arg, "2>") == 0) {
            arg = strtok(NULL, " "); //Get the error file name
            if (arg != NULL) {
                strcpy(error_file, arg);
                error_redirection = 1;
            }
        } else if (strcmp(arg, "2>>") == 0) {
            arg = strtok(NULL, " "); //Get the error file name (append)
            if (arg != NULL) {
                strcpy(error_file, arg);
                error_redirection = 2;
            }
        } else if (strcmp(arg, ">>") == 0) {
            arg = strtok(NULL, " "); //Get the output file name (append)
            if (arg != NULL) {
                strcpy(output_file, arg);
                output_redirection = 2;
            }
        }
        else{
            // store the token in the arguments array
            arguments[arg_counter] = strdup(arg);    
            arg_counter++;
        }
        arg = strtok(NULL, " ");
    }
    arguments[arg_counter] = NULL;

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

    int output_flags = O_WRONLY | O_CREAT;
    // Error redirection
    if (error_redirection) {
        int error_fd;
        if (error_redirection == 1) {
            // Redirect with truncation (`>`)
            error_fd = open(error_file, output_flags | O_TRUNC, 0666);
        } 
        else {
            // Redirect with append (`>>`)
            error_fd = open(error_file, output_flags | O_APPEND, 0666);
        }
        dup2(error_fd, STDERR_FILENO);
        close(error_fd);
    }

    // Output redirection
    //Determine whether to append or overwrite output
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

    // execute the command
    // printf("Executing command: %s\n", arguments[0]);
    int status = execvp(arguments[0], arguments);
    // printf("Command executed\n");
    //exit if failed to run command
    if (status == -1) {
        perror("Error executing command");
        exit(1);
    }
}

void execute(char** commands, int commands_count, int c_socket) {
    // array to store pipes
    int pipes[commands_count-1][2];
    int stdout_pipe[2];
    int stderr_pipe[2];

    if (pipe(stdout_pipe) < 0 || pipe(stderr_pipe) < 0) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }
    
    // iterate over all commands separated by pipes
    for (int i = 0; i < commands_count; i++) {
        // create pipe for all non-last commands
        if (i != commands_count - 1 && pipe(pipes[i]) == -1) {
            perror("pipe failed");
            exit(1);
        }
        pid_t pid = fork();

        if (pid < 0){
            perror("fork failed");
            exit(1);
        }
        else if(pid == 0){
            // close reading end of stderr and stdout pipes
            close(stdout_pipe[0]);
            close(stderr_pipe[0]);

            // replace stderr with writing end of stderr_pipe
            dup2(stderr_pipe[1], STDERR_FILENO);
            close(stderr_pipe[1]);

            //child process
            int input_fd = STDIN_FILENO;
            int output_fd = STDOUT_FILENO;

            // if the command is not the first one, redirect input to the reading end of pipe
            if (i != 0) {
                input_fd = pipes[i-1][0];
            } else {
                // if it is the first command, close the reading end of pipe
                if (commands_count > 1) close(pipes[i][0]);
            }

            // if the command is not the last one, redirect output to the writing end of pipe
            if (i != commands_count-1) {
                output_fd = pipes[i][1];
            } else {
                // if it is the last one, redirect to the client socket
                output_fd = stdout_pipe[1];
            }

            execute_command(commands[i], input_fd, output_fd);
        }
        else{
            // we need to close writing end from parent so that it doesn't block the program
            // if it wasn't closed child would wait for the parent to write something
            if (i != commands_count - 1) close(pipes[i][1]);
            // starting from the second command, we need to close reading end from parent of the previous pipe
            // because we won't use it anymore
            if (i != 0) close(pipes[i-1][0]);
            //parent process
            int status;
            waitpid(pid, &status, 0);
        }
    }

    // close writing end of stderr and stdout pipes
    close(stdout_pipe[1]);
    close(stderr_pipe[1]);

    // send the output to the client from stdout and stderr pipes
    char buffer[4096];
    bzero(buffer, sizeof(buffer));

    // int stdout_bytes = 0;

    // Read and send stdout to the client
    // while (1) {
        ssize_t stdout_bytes = read(stdout_pipe[0], buffer, sizeof(buffer));
        printf("Read %ld bytes from stdout\n", stdout_bytes);
        // if (nbytes <= 0) break;
        // Send buffer to the client
        // send(c_socket, buffer, sizeof(buffer), 0);
    // }
    // bzero(buffer, sizeof(buffer));

    // Read and send stderr to the client
    // while (1) {
        ssize_t nbytes = read(stderr_pipe[0], buffer + stdout_bytes, sizeof(buffer) - stdout_bytes);
        printf("Read %ld bytes from stderr\n", nbytes);
        // if (nbytes <= 0) break;
        // Send buffer to the client
        // send(c_socket, buffer, sizeof(buffer), 0);
    // }
    if (strlen(buffer) == 0) buffer[0] = '\0';

    send(c_socket, buffer, sizeof(buffer), 0);
    bzero(buffer, sizeof(buffer));
}