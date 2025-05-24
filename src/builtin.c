#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtin.h"
#include "command.h"
#include "shell_signal.h"

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

void handle_echo(char *argv[]) {
    if (!argv[1]) { // echo + nothing
        putchar('\n'); 
        return;
    }

    if (strcmp(argv[1], "$?") == 0 && argv[2] == NULL) { // echo $? only
        printf("%d\n", last_exit_status);
        return;
    }

    printf("%s", argv[1]);

    for (int i = 2; argv[i] != NULL; i++) {
        printf(" %s", argv[i]);
    }
    putchar('\n');
}

void handle_double_bang(char **last_cmd, int mode_indicator) {
    if (!*last_cmd || strlen(*last_cmd) == 0) {// when last_cmd is empty
        return;
    }

    if (mode_indicator) printf("%s\n", *last_cmd);
    process_cmd(*last_cmd, last_cmd, mode_indicator); // wrap up processing logics into a chunk, reuse in both main() & double_bang()
}
