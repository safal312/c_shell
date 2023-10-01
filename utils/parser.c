#include <stdlib.h>
#include <string.h>

#include "parser.h"

int parse_input(char* input, char** tokens) {
    char* arg = strtok(input, " \t\n");
    int counter = 0;

    while (arg != NULL) {
        tokens[counter] = strdup(arg);
        arg = strtok(NULL, " \t\n");
        counter++;
    }
    
    return counter;
}