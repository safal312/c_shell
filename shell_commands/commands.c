#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h> // for open

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
    // filenames for input and output redirection
    char input_file[50];
    char output_file[50];

    char delims[50];               // array to store the delimiters after parsing input
    int delim_counter = 0;
    for (int i = 0; command[i] != '\0'; i++) {
        if (command[i] == '>' || command[i] == '<') {
            delims[delim_counter] = command[i];
            // printf("Delimiter: %c\n", delims[delim_counter]);
            delim_counter++;
        }
    }

    char* original_command = strdup(command);
    char* redirect_present = strpbrk(command, "<>");
    char* main_command;

    if (redirect_present != NULL) {
        char* arg = strtok(command, "<>");
        main_command = strdup(arg);
    } else {
        main_command = strdup(command);
    }
    
    // split then on spaces to get the command and its arguments
    // printf("Main command: %s\n", main_command);
    // exit(0);
    char* arg = strtok(main_command, " ");
    while (arg != NULL) {
        // printf("Arg: %s\n", arg);
        arguments[arg_counter] = strdup(arg);    
        arg_counter++;
        arg = strtok(NULL, " ");
    }
    arguments[arg_counter] = NULL;
    
    int counter = 0;
    if (redirect_present != NULL) {
        arg = strtok(original_command, "<>");
        arg = strtok(NULL, "<>");
        // printf("Arg after redirect: %s\n", arg);
        while (arg != NULL) {
            trim(arg);
            if (delims[counter] == '<') {
                strcpy(input_file, arg);
                arg = strtok(NULL, "<>");
                input_redirection = 1;
            } else if (delims[counter] == '>'){
                strcpy(output_file, arg);
                arg = strtok(NULL, "<>");
                output_redirection = 1;
            }
            counter++;
        }
    }

    // printf("Input file: %s\n", input_file);
    // printf("Out file: %s\n", output_file);
    // exit(0);
    // int redirect_counter = 0;
    // char* arg = strtok(command, delim_chars);
    // while (arg != NULL) {
    //     if (delim_counter > 0 && delims[redirect_counter] == '<'){
    //         arg = strtok(NULL, " "); //Get the input file name
    //         if (arg != NULL) {
    //             strcpy(input_file, arg);
    //             input_redirection = 1;
    //         }
    //         redirect_counter++;
    //     }
    //     else if (delims[redirect_counter] == '>'){
    //         arg = strtok(NULL, " "); //Get the output file name
    //         printf("file: %s\n", arg);
    //         if (arg != NULL) {
    //             strcpy(output_file, arg);
    //             output_redirection = 1;
    //         }
    //         redirect_counter++;
    //     }
    //     else{
    //         arguments[arg_counter] = strdup(arg);    
    //         arg_counter++;
    //     }
    //     arg = strtok(NULL, delim_chars);
    // }
    // arguments[arg_counter] = NULL;

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
    //Determine whether to append or overwrite output
    int output_flags = O_WRONLY | O_CREAT;
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

void execute(char** commands, int commands_count) {
    int pipes[commands_count-1][2];

    for (int i = 0; i < commands_count; i++) {
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