#ifndef EXEC_H
#define EXEC_H

#include <stdbool.h>

// foreground: fork + exec + waitpid, background: fork + update job list + return immediately
void run_external_fg(char *argv[]);

void run_cmd(char *command, char *argv[], char **last_cmd, int mode_indicator, bool in_subshell); // run the command with argv[] and last_cmd

#endif
