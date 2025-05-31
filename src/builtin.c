#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "builtin.h"
#include "command.h"
#include "shell_signal.h"
#include "redirect.h"
#include "job.h"
#include "alias.h"

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
        printf("%i\n", last_exit_status);
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
}

void handle_fg(char *job_specifier) {
    if (job_specifier[0] != '%') {
        PANIC("Error in fg: fg command requires a job specifier starting with '%%'\n");
        last_exit_status = 1;
        return;
    }

    int job_id = atoi(job_specifier + 1); // skip the '%' character
    if (job_id <= 0) {
        PANIC("Error in fg: Invalid job id %i\n", job_id);
        last_exit_status = 1;
        return;
    }

    Job *job = find_by_jid(job_id);
    if (!job) {
        PANIC("Error in fg: No such job with job id %i\n", job_id);
        last_exit_status = 1;
        return;
    }

    // block SIGCHLD during bringing the job to foreground
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    printf("%s\n", job->cmd);
    fflush(stdout);

    // bring the job to the foreground
    fg_pgid = job->pgid;
    tcsetpgrp(STDIN_FILENO, fg_pgid);

    // restore the job if stopped
    if (job->state == STOPPED) {
        kill(-job->pgid, SIGCONT); // send SIGCONT to the process group
        job->state = RUNNING;
    }

    int status;
    waitpid(-job->pgid, &status, WUNTRACED);
    if (WIFEXITED(status)) {
        remove_job(job->pgid);
    } else if (WIFSIGNALED(status)) {
        last_exit_status = WTERMSIG(status);
        write(STDOUT_FILENO, "\n", 1); // let next prompt start on a new line
        remove_job(job->pgid);
    } else if (WIFSTOPPED(status)) {
        job->state = STOPPED;
        printf("\n[%i]+ Stopped\t%s\n", job_id, job->cmd);
        fflush(stdout);
        last_exit_status = WSTOPSIG(status);
    }

    tcsetpgrp(STDIN_FILENO, getpid()); // give terminal back to parent process
    fg_pgid = 0; // reset foreground process group ID

    sigprocmask(SIG_SETMASK, &oldmask, NULL); // restore the previous signal mask
    last_exit_status = 0;
}

void handle_bg(char *job_specifier) {
    if (job_specifier[0] != '%') {
        PANIC("Error in bg: bg command requires a job specifier starting with '%%'\n");
        last_exit_status = 1;
        return;
    }

    int job_id = atoi(job_specifier + 1);
    if (job_id <= 0) {
        PANIC("Error in bg: invalid job id %i\n", job_id);
        last_exit_status = 1;
        return;
    }

    Job *job = find_by_jid(job_id);
    if (!job) {
        PANIC("Error in bg: no such job with job id %i\n", job_id);
        last_exit_status = 1;
        return;
    }

    if (job->state == STOPPED) {
        kill(-job->pgid, SIGCONT);
        job->state = RUNNING;
        printf("[%i]+ %s &\n", job_id, job->cmd);
        last_exit_status = 0;
    } else {
        PANIC("Error in bg: job %i already running\n", job_id);
        last_exit_status = 1;
    }
}

void handle_alias(char *argv[]){
    if (!argv[1]) { // no alias name given
        list_aliases();
        last_exit_status = 0;
        return;
    }

    char *arg = argv[1];
    char *equal_sign = strchr(arg, '=');
    if (!equal_sign) { // no '=' founded
        PANIC("Error in alias: usage: alias name='value'\n");
        last_exit_status = 1;
        return;
    }

    size_t name_len = equal_sign - arg;
    char name_buf[256];
    if (name_len >= sizeof(name_buf)) {
        PANIC("Error in alias: alias name too long\n");
        last_exit_status = 1;
        return;
    }
    memcpy(name_buf, arg, name_len);
    name_buf[name_len] = '\0'; // null-terminate the alias name

    char *val = equal_sign + 1; // skip the '=' character
    size_t val_len = strlen(val);
    if ((val[0] == '\'' && val[val_len - 1] == '\'') || (val[0] == '"' && val[val_len - 1] == '"')) {
        char temp_buf[256];
        if (val_len >= sizeof(temp_buf)) {
            PANIC("Error in alias: alias value too long\n");
            last_exit_status = 1;
            return;
        }

        memcpy(temp_buf, val + 1, val_len - 2);
        temp_buf[val_len - 2] = '\0'; // null-terminate the alias value
        add_or_update_alias(name_buf, temp_buf);
    } else {
        add_or_update_alias(name_buf, val);
    }
    last_exit_status = 0;
}

void handle_unalias(char *argv[]) {
    if (!argv[1]) {
        PANIC("Error in unalias: usage: unalias name\n");
        last_exit_status = 1;
        return;
    }

    int result = remove_alias(argv[1]);
    if (result < 0) {
        PANIC("Error in unalias: no such alias '%s'\n", argv[1]);
        last_exit_status = 1;
    } else {
        last_exit_status = 0;
    }
}