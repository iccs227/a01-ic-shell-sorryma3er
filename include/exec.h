#ifndef EXEC_H
#define EXEC_H

#include <stdbool.h>

// foreground: fork + exec + waitpid, background: fork + update job list + return immediately
void run_external_fg(char *argv[]);

//split the command into argv[] + NULL sentinel
void split_args(char *to_split, char *argv[]);

#endif
