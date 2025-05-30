#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "command.h"
#include "builtin.h"
#include "exec.h"
#include "shell_signal.h"
#include "redirect.h"
#include "job.h"

void process_cmd(char *command, char **last_cmd, int mode_indicator) {
    if (is_repeat_bg(command, *last_cmd)) {
        fprintf(stderr, "icsh: parse error near '&'\n");
        last_exit_status = 1; // set exit status to 1 for error
        return;
    }

    char cmd_no_amp[MAX_CMD_BUFFER];
    strcpy(cmd_no_amp, command);

    bool is_background_pre = strip_ampersand(cmd_no_amp); // remove trailing '&' on raw command

    /* Double bang check and expansion */
    char expanded_cmd[MAX_CMD_BUFFER];
    bool is_double_bang = (strcmp(cmd_no_amp, "!!") == 0);
    if (is_double_bang) {
        if (!*last_cmd) {
            fprintf(stderr, "No previous command\n");
            return;
        }
        handle_double_bang(last_cmd, mode_indicator);
        strcpy(expanded_cmd, *last_cmd);
    } else {
        strcpy(expanded_cmd, cmd_no_amp);
    }

    // update last_cmd only if not double bang
    if (!is_double_bang) {
        if (*last_cmd) free(*last_cmd);
        *last_cmd = strdup(command);
    }

    // reprocess the expanded command
    bool is_background_aft = strip_ampersand(expanded_cmd); // remove trailing '&' on expanded command
    bool is_background = is_background_pre || is_background_aft; // if either pre or post processing has '&', then its a background process

    char *argv[MAX_ARGS + 1]; // +1 for NULL sentinel
    char to_split[MAX_CMD_BUFFER];
    strcpy(to_split, expanded_cmd);
    split_args(to_split, argv); // split on the cleaned command without '&'

    if (is_background) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed in background process: ");
            last_exit_status = 1;
            return;
        } else if (pid == 0) { // child process run in background
            setpgid(getpid(), getpid());

            run_cmd(expanded_cmd, argv, last_cmd, mode_indicator, true);
            exit(last_exit_status);
        } else {
            setpgid(pid, pid);

            int job_id = add_job(pid, expanded_cmd, RUNNING);
            last_exit_status = 0; // background job always exit with 0
            printf("[%i] %i\n", job_id, pid);
            fflush(stdout); // flush the output buffer to ensure the job id is printed immediately
            return;
        }
    } else { // foreground process
        run_cmd(expanded_cmd, argv, last_cmd, mode_indicator, false);
    }

}

void split_args(char *to_split, char *argv[]){
    int idx = 0;
    char *token = strtok(to_split, " "); // get the first word
    while(token && idx < MAX_ARGS) {
        argv[idx] = token; // store the token in the array
        token = strtok(NULL, " "); // get the next word
        idx++;
    }
    argv[idx] = NULL; // add NULL sentinel
}

bool is_repeat_bg(char *command, char *last_cmd) { // Note: "!!&" not supported, only "!! &" is supported
    if (!command || !last_cmd) return false;

    if (strncmp(command, "!!", 2) != 0) return false;

    const char *ptr = command + 3; // skip "!! "

    if (*ptr != '&') return false;

    ++ptr; // skip '&'
    if (*ptr != '\0') return false;

    size_t len = strlen(last_cmd);
    if (len == 0) return false;
    return last_cmd[len - 1] == '&';
}

bool strip_ampersand(char *command) { // return true if command ends with '&', false otherwise
    bool is_background = false;

    if (command[strlen(command) - 1] == '&') {
        is_background = true;
        command[strlen(command) - 1] = '\0'; // remove '&'

        // remove trailing space before '&'
        size_t len = strlen(command);
        while (len > 0 && command[len - 1] == ' ') {
            command[len - 1] = '\0';
            len--;
        }
    }

    return is_background;
}