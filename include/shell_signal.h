#ifndef SHELL_SIGNAL_H
#define SHELL_SIGNAL_H

#include <sys/types.h>

void handle_sigint(int sig_num);
void handle_sigtstp(int sig_num);
void handle_sigchld(int sig_num);
void install_signal_handler(void);

extern pid_t fg_pgid;
extern int last_exit_status;
extern const char *prompt;

#endif