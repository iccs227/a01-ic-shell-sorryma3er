/* ICCS227: Project 1: icsh
 * Name: Haicheng Wang
 * StudentID: 6580244
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CMD_BUFFER 255 // maximum length of input; final

//refactor out by features to make the main() cleaner
void handle_exit(char *buffer);
void handle_echo(char *buffer);

int main() {
    char buffer[MAX_CMD_BUFFER]; // where we store the user input

    printf("Starting IC shell! Type commands down below\n"); // welcome message before REPL

    //REPL
    while (1) {
        printf("icsh $ "); // prompt symbol
        
        if (!fgets(buffer, MAX_CMD_BUFFER, stdin)) break; // read and put the input into buffer
        buffer[strcspn(buffer, "\n")] = '\0'; // remove the trailing newline '\n' character

        if (strlen(buffer) == 0) continue; // if input is empty, go to next round

        printf("you said: %s\n", buffer);

        char buffer_cp[MAX_CMD_BUFFER];
        strcpy(buffer_cp, buffer);// make a copy here for func use later
        printf("copy is: %s\n", buffer_cp);
        char *token = strtok(buffer_cp, " "); // tokenize from the copy wont affect ori buffer

        if (strcmp(token, "exit") == 0) {
            handle_exit(buffer); 
        } else if (strcmp(token, "echo") == 0) {
            handle_echo(buffer);
        } else {
            printf("bad command!\n");
        }
    }

    return 0;
}

void handle_exit(char *buffer) {
    char copy[MAX_CMD_BUFFER];
    strcpy(copy, buffer);

    //skip the exit word
    strtok(copy, " ");
    char *code_str = strtok(NULL, " ");
    printf("code char is %s\n", code_str);

    if (!code_str) { //no exit code
        printf("here\n");
        printf("Bye!\n");
        exit(0);
    } else {
        int code = atoi(code_str) % 256;
        if (code < 0) code += 256;
        
        printf("Bye!\n");
        exit(code);
    }
}

void handle_echo(char *buffer) {
    char *to_print = buffer + 5; //buffer here is an addr. +5 will skip 'echo '
    if (strlen(to_print) > 0) {
        printf("%s\n", to_print);
    } else {
        printf("\n");
    }
}
