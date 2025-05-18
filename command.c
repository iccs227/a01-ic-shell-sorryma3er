#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "builtin.h"

#define MAX_CMD_BUFFER 255 // maximum length of input; final

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
        printf("bad command!\n");
    }

    if (strcmp(command, "!!") != 0) { // update the last_cmd field, !! shouldnt come with any other char

        if (*last_cmd) free(*last_cmd); // prevent mem leak

        //printf("last command before update is %s\n", command);
        *last_cmd = strdup(command);
    }
}