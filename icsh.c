/* ICCS227: Project 1: icsh
 * Name: Haicheng Wang
 * StudentID: 6580244
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CMD_BUFFER 255 // maximum length of input; final

int main() {
    char buffer[MAX_CMD_BUFFER]; // where we store the user input

    printf("Starting IC shell! Type commands down below\n"); // welcome message before REPL

    while (1) {
        printf("icsh $ "); // prompt symbol
        
        if (!fgets(buffer, MAX_CMD_BUFFER, stdin)) break; // read and put the input into buffer

        buffer[strcspn(buffer, "\n")] = '\0'; // remove the trailing newline '\n' character

        if (strlen(buffer) == 0) continue; // if input is empty, go to next round

        printf("you said: %s\n", buffer);

        //handle normal exit
        if (strcmp(buffer, "exit") == 0) {
            printf("Bye!\n");
            exit(0);
        }

        //handle exit wwith code
        char *token = strtok(buffer, " ");
        if (token && strcmp(token, "exit") == 0) {
            int code = 0;
            char *code_char = strtok(NULL, " "); // passing NULL to it will continue parse the ori string

            if (code_char) code = atoi(code_char) % 256;
            if (code < 0) code += 256; // deal with neg exit code

            printf("Bye!\n");
            printf("exit code is: %i\n", code);
            exit(code);
        }
        

    }

    return 0;
}
