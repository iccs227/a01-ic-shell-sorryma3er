#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>

void process_cmd(char *command, char **last_cmd, int mode_indicator); // mode_indicator: 0 => script mode, 1 => interactive mode

void run_cmd(char *raw_cmd, char *cmd_no_amp, char *argv[], char **last_cmd, int mode_indicator, bool in_subshell); // run the command with argv[] and last_cmd

bool is_repeat_bg(char *command, char *last_cmd); // check if the current cmd and last cmd both contain '&' at the end, if so, return true

#endif