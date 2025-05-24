#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "command.h"
#include "builtin.h"
#include "exec.h"
#include "shell_signal.h"
#include "redirect.h"

#define MAX_CMD_BUFFER 255 // maximum length of input; final
#define MAX_ARGS 16 // assume the external command contains no more than 16 words; final

void process_cmd(char *command, char **last_cmd, int mode_indicator) {
    char cmd_copy[MAX_CMD_BUFFER];// make a cp for command
    strcpy(cmd_copy, command);

    char *argv[MAX_ARGS + 1]; // +1 for NULL sentinel
    char to_split[MAX_CMD_BUFFER]; // since cmd_copy is alr tokenized and contains \0 inside
    strcpy(to_split, command);
    split_args(to_split, argv);

    /*should do Redirect register here so it works for buildtin as well */
    int saved_out = dup(STDOUT_FILENO);
    int saved_in = dup(STDIN_FILENO);

    Redirect redirect;
    if (parse_redirect(argv, &redirect) < 0) { // parse for redirect failed
        last_exit_status = 1; 
        close(saved_in);
        close(saved_out);
        return;
    }
    if (apply_redirect(&redirect) < 0) { // apply redirect failed
        last_exit_status = 1; 
        free_redirect(&redirect);
        close(saved_in);
        close(saved_out);
        return;
    }
    free_redirect(&redirect);

    char *token = strtok(cmd_copy, " "); // get first token from copy, so ori command field will stay the same
    if (!token) return;

    if (strcmp(token, "exit") == 0) {
        handle_exit(command, mode_indicator);
    } else if (strcmp(token, "echo") == 0) {
        handle_echo(argv);
        last_exit_status = 0; //builtin command exit with 0
    } else if (strcmp(command, "!!") == 0) {
        handle_double_bang(last_cmd, mode_indicator);
        last_exit_status = 0; //builtin command exit with 0
    } else {
        /*External command handle: */
        if (argv[0]) run_external(argv);
    }

    if (strcmp(command, "!!") != 0) { // update the last_cmd field, !! shouldnt come with any other char
        if (*last_cmd) free(*last_cmd); // prevent mem leak
        *last_cmd = strdup(command);
    }

    //restore the fd back to saved ones here:
    if (dup2(saved_in, STDIN_FILENO) < 0) {
        perror("dup2 restore stdin failed");
        exit(1);
    }
    if (dup2(saved_out, STDOUT_FILENO) < 0) {
        perror("dup2 restore stdout failed");
        exit(1);
    }
    close(saved_in);
    close(saved_out);
}