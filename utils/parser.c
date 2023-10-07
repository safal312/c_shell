#include <stdlib.h>
#include <string.h>

#include "parser.h"

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

int parse_input(char* input, char** commands, char* delims) {
    trim(input);
    char* input_copy = strdup(input);
    char* arg = strtok(input, "|><");
    int counter = 0;
    int delim_counter = 0;

    while (arg != NULL) {
        trim(arg);
        commands[counter] = strdup(arg);
        arg = strtok(NULL, "|><");
        counter++;
    }
    commands[counter] = NULL;
    
    for (int i = 0; input_copy[i] != '\0'; i++) {
        if (input_copy[i] == '|' || input_copy[i]=='<' || input_copy[i]=='>') {
            delims[delim_counter] = input_copy[i];
            delim_counter++;
        }
    }
    
    return counter;
}