#ifndef BUILTIN_H
#define BUILTIN_H

void handle_exit(char *buffer, int mode_indicator);
void handle_echo(char *argv[]);
void handle_double_bang(char **last_cmd, int mode_indicator);

#endif
