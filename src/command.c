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

#define MAX_CMD_BUFFER 255 // maximum length of input; final
#define MAX_ARGS 16 // assume the external command contains no more than 16 words; final

void process_cmd(char *command, char **last_cmd, int mode_indicator) {
    if (is_repeat_bg(command, *last_cmd)) {
        fprintf(stderr, "icsh: parse error near '&'\n");
        last_exit_status = 1; // set exit status to 1 for error
        return;
    }

    char cmd_no_amp[MAX_CMD_BUFFER];
    strcpy(cmd_no_amp, command);

    /* Detect backgroud sign '&' and parse */
    bool is_background = false; // flag to indicate if the command is run in background
    if (cmd_no_amp[strlen(cmd_no_amp) - 1] == '&') {
        is_background = true;
        cmd_no_amp[strlen(cmd_no_amp) - 1] = '\0'; // remove '&'

        // remove trailing space before '&'
        size_t len = strlen(cmd_no_amp);
        while (len > 0 && cmd_no_amp[len - 1] == ' ') {
            cmd_no_amp[len - 1] = '\0';
            len--;
        }
    }

    /* Double bang check */
    if (strcmp(cmd_no_amp, "!!") != 0) {
        if (*last_cmd) free(*last_cmd);
        *last_cmd = strdup(command); // update last_cmd with the raw command
    } else {
        // pull out handle_double_bang function
        handle_double_bang(last_cmd, mode_indicator);
        return; // if its a double bang, then no need to run the command
    }

    char *argv[MAX_ARGS + 1]; // +1 for NULL sentinel
    char to_split[MAX_CMD_BUFFER];
    strcpy(to_split, cmd_no_amp);
    split_args(to_split, argv); // split on the cleaned command without '&'

    if (is_background) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed in background process: ");
            last_exit_status = 1;
            return;
        } else if (pid == 0) { // child process run in background
            setpgid(getpid(), getpid());

            run_cmd(command, cmd_no_amp, argv, last_cmd, mode_indicator, true);
            exit(last_exit_status);
        } else {
            setpgid(pid, pid);

            int job_id = add_job(pid, cmd_no_amp);
            last_exit_status = 0; // background job always exit with 0
            printf("[%i] %i\n", job_id, pid);
            fflush(stdout); // flush the output buffer to ensure the job id is printed immediately
            return;
        }
    } else { // foreground process
        run_cmd(command, cmd_no_amp, argv, last_cmd, mode_indicator, false);
    }

}


void run_cmd(char *raw_cmd, char *cmd_no_amp, char *argv[], char **last_cmd, int mode_indicator, bool in_subshell) {
    /* Redirect register here so it works for buildtin as well */
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

    char cmd_copy[MAX_CMD_BUFFER];
    strcpy(cmd_copy, raw_cmd);
    char *token = strtok(cmd_copy, " "); // get first token from copy, so ori command field will stay the same
    if (!token) {
        close(saved_in);
        close(saved_out);
        return; // no command to run
    }

    if (strcmp(token, "exit") == 0) {
        handle_exit(raw_cmd, mode_indicator);
    } else if (strcmp(token, "echo") == 0) {
        handle_echo(argv);
        last_exit_status = 0; //builtin command exit with 0
    } else {
        if (in_subshell) { // if in subshell, then rn its running external cmd in child process at the bg
            execvp(argv[0], argv);

            fprintf(stderr, "running external command in the background failed: %s, %s\n", argv[0], strerror(errno)); // reach here only on failure
            exit(1);
        } else { // not in subshell, so run external command in foreground
            if (argv[0]) run_external_fg(argv);
        }
    }

    // restore the fd back to saved ones here:
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