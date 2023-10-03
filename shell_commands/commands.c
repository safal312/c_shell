#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "commands.h"

void execute_command(char** arguments) {
    int status = execvp(arguments[0], arguments);

    //exit if failed to run command
    if (status == -1) {
        perror("Error executing command");
        exit(1);
    }
}