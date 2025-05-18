#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtin.h"
#include "command.h"

#define MAX_CMD_BUFFER 255 // maximum length of input; final

void handle_exit(char *buffer, int mode_indicator) {
    char copy[MAX_CMD_BUFFER];
    strcpy(copy, buffer);

    //skip the exit word
    strtok(copy, " ");
    char *code_str = strtok(NULL, " ");

    if (!code_str) { //no exit code
        if (mode_indicator) printf("bye\n");
        exit(0);
    } else {
        int code = atoi(code_str) % 256;
        if (code < 0) code += 256;
        
        if (mode_indicator) printf("bye\n");
        exit(code);
    }
}

void handle_echo(char *buffer) {
    char *to_print = buffer + 5; // buffer here is an addr. +5 will skip 'echo '
    if (strlen(to_print) > 0) {
        printf("%s\n", to_print);
    } else {
        printf("\n");
    }
}

void handle_double_bang(char **last_cmd, int mode_indicator) {
    if (!*last_cmd || strlen(*last_cmd) == 0) {// when last_cmd is empty
        return;
    }

    if (mode_indicator) printf("%s\n", *last_cmd);
    process_cmd(*last_cmd, last_cmd, mode_indicator); // wrap up processing logics into a chunk, reuse in both main() & double_bang()
}
