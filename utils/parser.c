#include <stdlib.h>
#include <string.h>

#include "parser.h"

// https://saturncloud.io/blog/best-algorithm-to-strip-leading-and-trailing-spaces-in-c/
// helper function that remooves trailing and leading whitespace characters
void trim(char* str) {
    int start = 0;
    int end = strlen(str) - 1;

    while (str[start] == ' ' || str[start] == '\t' || str[start] == '\n') {
        start++;
    }

    while (str[end] == ' ' || str[end] == '\t' || str[end] == '\n') {
        end--;
    }

    str[end + 1] = '\0';

    // Move the trimmed substring to the beginning of the string
    memmove(str, str + start, end - start + 2);
}

int parse_input(char* input, char** commands) {
    // remove trailing or leading whitespace or newline characters
    trim(input);
    // initially we only split on pipes
    char* arg = strtok(input, "|");
    int counter = 0;

    while (arg != NULL) {
        // remove trailing or leading whitespace or newline characters
        trim(arg);
        commands[counter] = strdup(arg);
        arg = strtok(NULL, "|");
        counter++;
    }
    commands[counter] = NULL;
    
    return counter;
}