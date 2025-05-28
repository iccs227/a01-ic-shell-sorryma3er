#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "shell_signal.h"
#include "job.h"

pid_t fg_pgid = 0;
int last_exit_status = 0;

void handle_sigint(int sig_num) {
    if (fg_pgid > 0) kill(-fg_pgid, SIGINT);
    // resend the signal to all the process with group id == fg_pgid
}

void handle_sigtstp(int sig_num) {
    if (fg_pgid > 0) kill(-fg_pgid, SIGTSTP);
}

void handle_sigchld(int sig_num) {
    pid_t pid;
    int status;
    bool printed_done = false;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        Job *job = find_by_pgid(pid);
        if (job) {
            printf("[%i]+  Done\t%s\n", job->job_id, job->cmd);
            remove_job(pid); // remove the job from the job list
            printed_done = true;
        }
    }

    if (printed_done && fg_pgid == 0) {
        printf("icsh $ ");
        fflush(stdout);
    }
}

void install_signal_handler(void) {
    struct sigaction sa_int, sa_tstp, sa_chld;

    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sa_int.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa_int, NULL);

    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sa_tstp.sa_handler = handle_sigtstp;
    sigaction(SIGTSTP, &sa_tstp, NULL);

    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP; // only reap terminated children, ignore stopped children
    sa_chld.sa_handler = handle_sigchld;
    sigaction(SIGCHLD, &sa_chld, NULL);

    signal(SIGTTOU, SIG_IGN); // ignore both SIGTTOU & SIGTTIN, so when shell is under bg, read and write wont stop the process
    signal(SIGTTIN, SIG_IGN);
}