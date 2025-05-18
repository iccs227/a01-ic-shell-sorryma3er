#ifndef EXEC_H
#define EXEC_H

//fork + exec + waitpid
void run_external(char *argv[]);

//split the command into argv[] + NULL sentinel
void split_args(char *to_split, char *argv[]);

#endif
