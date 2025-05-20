#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "exec.h"
#include "shell_signal.h"
#include "redirect.h"

#define MAX_ARGS 16 // assume the external command contains no more than 16 words; final

void run_external(char *argv[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed: ");
        last_exit_status = -1;
        return;
    } else if (pid == 0) { // child process
        setpgid(getpid(), getpid());

        signal(SIGINT, SIG_DFL); // set the child process signal handler back to default, so it reacts to SIGINT & SIGTSTP
        signal(SIGTSTP, SIG_DFL);

        Redirect redirect;
        if (parse_redirect(argv, &redirect) < 0) exit(1); // parse failed
        if (apply_redirect(&redirect) < 0) exit(1); // apply redirect failed
        free_redirect(&redirect);

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
        }

        tcsetpgrp(STDIN_FILENO, getpid()); // give terminal back to parent process
        fg_pgid = 0; // set forerground id back to 0, so handler no more redirect to pg
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