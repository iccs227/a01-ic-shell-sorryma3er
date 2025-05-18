#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "builtin.h"
#include "exec.h"

#define MAX_CMD_BUFFER 255 // maximum length of input; final
#define MAX_ARGS 16 // assume the external command contains no more than 16 words; final

void process_cmd(char *command, char **last_cmd, int mode_indicator) {
    char cmd_copy[MAX_CMD_BUFFER];// make a cp for command
    strcpy(cmd_copy, command);

    char *token = strtok(cmd_copy, " ");// get first token from copy, so ori command field will stay the same
    if (!token) return;

    if (strcmp(token, "exit") == 0) {
        handle_exit(command, mode_indicator);
    } else if (strcmp(token, "echo") == 0) {
        handle_echo(command);
    } else if (strcmp(command, "!!") == 0) {
        handle_double_bang(last_cmd, mode_indicator);
    } else {
        /*External command handle: */
        char *argv[MAX_ARGS + 1]; // +1 for NULL sentinel
 
        char to_split[MAX_CMD_BUFFER]; // since cmd_copy is alr tokenized and contains \0 inside
        strcpy(to_split, command);

        split_args(to_split, argv); // argv[i] points to something that initialize in this scope, prevent dangling pointer

        if (argv[0]) run_external(argv);
    }

    if (strcmp(command, "!!") != 0) { // update the last_cmd field, !! shouldnt come with any other char

        if (*last_cmd) free(*last_cmd); // prevent mem leak

        //printf("last command before update is %s\n", command);
        *last_cmd = strdup(command);
    }
}