#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "commands.h"

void execute_command(char** arguments) {

    execvp(arguments[0], arguments);
}