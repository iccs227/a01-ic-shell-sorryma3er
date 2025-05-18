#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "signal.h"

pid_t fg_pgid = 0;
int last_exit_status = 0;

void handle_sigint(int sig_num) {
    if (fg_pgid > 0) kill(-fg_pgid, SIGINT);
    // resend the signal to all the process with group id == fg_pgid
}

void handle_sigtstp(int sig_num) {
    if (fg_pgid > 0) kill(-fg_pgid, SIGTSTP);
}

void install_signal_handler(void) {
    struct sigaction sa_int, sa_tstp;

    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sa_int.sa_handler = handle_sigint;
    sigaction(SIGINT, &sa_int, NULL);

    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sa_tstp.sa_handler = handle_sigtstp;
    sigaction(SIGTSTP, &sa_tstp, NULL);

    signal(SIGTTOU, SIG_IGN); // ignore both SIGTTOU & SIGTTIN, so when shell is under bg, read and write wont stop the process
    signal(SIGTTIN, SIG_IGN);
}