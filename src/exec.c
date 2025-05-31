#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "exec.h"
#include "shell_signal.h"
#include "redirect.h"
#include "command.h"
#include "builtin.h"
#include "job.h"

void run_external_fg(char *argv[], char *command) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed: ");
        last_exit_status = 1;
        return;
    } else if (pid == 0) { // child process
        setpgid(getpid(), getpid());

        signal(SIGINT, SIG_DFL); // set the child process signal handler back to default, so it reacts to SIGINT & SIGTSTP
        signal(SIGTSTP, SIG_DFL);

        execvp(argv[0], argv);

        fprintf(stderr, "running external command failed: %s, %s\n", argv[0], strerror(errno)); // reach here only on failure
        exit(1);
    } else { // parent process
        setpgid(pid, pid); // prevent race condtion here?

        fg_pgid = pid; // let the signal handler now can direct the signal to foreground child process

        tcsetpgrp(STDIN_FILENO, pid); // give terminal control to the foreground process group

        int status;
        waitpid(pid, &status, WUNTRACED); // while waiting trace the exit status of child

        if (WIFEXITED(status)) { // capture exit/stop status
            last_exit_status = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            last_exit_status = WTERMSIG(status); // let exit status equals to the # of signal cause child to terminate
            write(STDOUT_FILENO, "\n", 1);
        } else if (WIFSTOPPED(status)) {
            last_exit_status = WSTOPSIG(status); // the # of signals cause child to stop
            write(STDOUT_FILENO, "\n", 1);

            // add the stopped job to the job list
            int job_id = add_job(pid, command, STOPPED);
            printf("[%d]+  Stopped\t\t%s\n", job_id, command);
        }

        tcsetpgrp(STDIN_FILENO, getpid()); // give terminal back to parent process
        fg_pgid = 0; // set forerground id back to 0, so handler no more redirect to pg
    }
}

void run_cmd(char *command, char *argv[], char **last_cmd, int mode_indicator, bool in_subshell) {
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
    strcpy(cmd_copy, command);
    char *token = strtok(cmd_copy, " "); // get first token from copy, so ori command field will stay the same
    if (!token) {
        close(saved_in);
        close(saved_out);
        return; // no command to run
    }

    if (strcmp(token, "exit") == 0) {
        handle_exit(command, mode_indicator);
    } else if (strcmp(token, "echo") == 0) {
        handle_echo(argv);
        last_exit_status = 0; //builtin command exit with 0
    } else if (strcmp(token, "jobs") == 0) {
        list_jobs();
        last_exit_status = 0;
    } else if (strcmp(token, "fg") == 0) {
        if (!argv[1]) {
            PANIC("Error in fg: fg command requires a job specifier\n");
            last_exit_status = 1;
            close(saved_in);
            close(saved_out);
            return;
        }
        handle_fg(argv[1]);
    } else if (strcmp(token, "bg") == 0) {
        if (!argv[1]) {
            PANIC("Error in bg: bg command requires a job specifier\n");
            last_exit_status = 1;
            close(saved_in);
            close(saved_out);
            return;
        }
        handle_bg(argv[1]);
    } else if (strcmp(token, "alias") == 0) {
        handle_alias(argv);
    } else if (strcmp(token, "unalias") == 0) {
        handle_unalias(argv);
    } else {
        if (in_subshell) { // if in subshell, then rn its running external cmd in child process at the bg
            execvp(argv[0], argv);

            fprintf(stderr, "running external command in the background failed: %s, %s\n", argv[0], strerror(errno)); // reach here only on failure
            exit(1);
        } else { // not in subshell, so run external command in foreground
            if (argv[0]) run_external_fg(argv, command);
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