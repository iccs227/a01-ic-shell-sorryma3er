/* ICCS227: Project 1: icsh
 * Name: Haicheng Wang
 * StudentID: 6580244
 */

#include <stdio.h>
#include <string.h>

#define MAX_CMD_BUFFER 255 // maximum length of input

int main() {
    char buffer[MAX_CMD_BUFFER]; // where we store the user input

    printf("Starting IC shell! Type commands down below\n"); // welcome message before REPL

    while (1) {
        printf("icsh $ "); // prompt symbol
        
        if(!fgets(buffer, MAX_CMD_BUFFER, stdin)) break; // read and put the input into buffer

        buffer[strcspn(buffer, "\n")] = '\0'; // remove the trailing newline '\n' character

        if(strlen(buffer) == 0) continue; // if input is empty, go to next round
        
        printf("you said: %s\n", buffer);
    }
}
