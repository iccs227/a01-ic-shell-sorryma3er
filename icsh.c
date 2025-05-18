/* ICCS227: Project 1: icsh
 * Name: Haicheng Wang
 * StudentID: 6580244
 */

 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include "command.h"
 #include "script.h"

 #define MAX_CMD_BUFFER 255 // maximum length of input; final

 int main(int argc, char *argv[]) {
    char buffer[MAX_CMD_BUFFER]; // where we store the user input
    char *last_cmd = NULL;

    if (argc == 2) return run_script(argv[1]); // if two args are given, then its script mode

    printf("Starting IC shell! Type commands down below\n"); // welcome message before REPL

    //REPL
    while (1) {
        printf("icsh $ "); // prompt symbol
        
        if (!fgets(buffer, MAX_CMD_BUFFER, stdin)) break; // read and put the input into buffer
        buffer[strcspn(buffer, "\n")] = '\0'; // remove the trailing newline '\n' character

        if (strlen(buffer) == 0) continue; // if input is empty, go to next round

        process_cmd(buffer, &last_cmd, 1); // mode_indicator: 1 => interactive mode
    }

    if (last_cmd) free(last_cmd);
    return 0;
 }

