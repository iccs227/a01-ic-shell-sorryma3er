#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>

#define MAX_CMD_BUFFER 255 // maximum length of input; final
#define MAX_ARGS 16 // assume the external command contains no more than 16 words; final

void process_cmd(char *command, char **last_cmd, int mode_indicator); // mode_indicator: 0 => script mode, 1 => interactive mode

bool is_repeat_bg(char *command, char *last_cmd); // check if the current cmd and last cmd both contain '&' at the end, if so, return true

bool strip_ampersand(char *command); // remove the trailing '&' from the command and return true if it was present

void split_args(char *to_split, char *argv[]); //split the command into argv[] + NULL sentinel

#endif