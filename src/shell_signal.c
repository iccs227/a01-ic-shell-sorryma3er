#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
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

void handle_sigchld(int sig) {
    pid_t pid;
    int status;
    char buf[256];
    bool printed = false;

    Job *job = jobs_head;
    while (job) {
        if (job->pgid == fg_pgid) {
            job = job->next;   // skip the foreground job entirely
            continue;
        }

        pid = waitpid(-job->pgid, &status, WNOHANG|WUNTRACED);
        if (pid > 0) {
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                // clear user input, still dont know how to save current user input
                tcflush(STDIN_FILENO, TCIFLUSH);
                write(STDOUT_FILENO, "\r\x1b[2K", 5);

                int n = snprintf(buf, sizeof(buf), "[%i]+ Done\t%s\n", job->job_id, job->cmd);
                write(STDOUT_FILENO, buf, n);
                remove_job(job->pgid);
                printed = true;

                job = jobs_head;
                continue;
            } else if (WIFSTOPPED(status)) {
                job->state = STOPPED;
                int n = snprintf(buf, sizeof(buf), "[%i]+ Stopped\t%s\n", job->job_id, job->cmd);
                write(STDOUT_FILENO, buf, n);
                printed = true;
            }
        }
        job = job->next;
    }

    if (printed && fg_pgid == 0) { // prevent job completion message printed on the same line as the prompt
        write(STDOUT_FILENO, prompt, strlen(prompt));
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
    sa_chld.sa_flags = SA_RESTART;
    sa_chld.sa_handler = handle_sigchld;
    sigaction(SIGCHLD, &sa_chld, NULL);

    signal(SIGTTOU, SIG_IGN); // ignore both SIGTTOU & SIGTTIN, so when shell is under bg, read and write wont stop the process
    signal(SIGTTIN, SIG_IGN);
}