/* ICCS227: Project 1: icsh
 * Name: Haicheng Wang
 * StudentID: 6580244
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_CMD_BUFFER 255 // maximum length of input; final

//refactor out by features to make the main() cleaner
void handle_exit(char *buffer);
void handle_echo(char *buffer);
void handle_double_bang(char **last_cmd);
void process_cmd(char *command, char **last_cmd);
int run_script(char *path);

int main(int argc, char **argv) { //argc for count, argv is array of argument
    char buffer[MAX_CMD_BUFFER]; // where we store the user input
    char *last_cmd = NULL;

    printf("Starting IC shell! Type commands down below\n"); // welcome message before REPL

    if (argc == 2) return run_script(argv[1]);

    //REPL
    while (1) {
        printf("icsh $ "); // prompt symbol
        
        if (!fgets(buffer, MAX_CMD_BUFFER, stdin)) break; // read and put the input into buffer
        buffer[strcspn(buffer, "\n")] = '\0'; // remove the trailing newline '\n' character

        if (strlen(buffer) == 0) continue; // if input is empty, go to next round

        process_cmd(buffer, &last_cmd);
    }

    if (last_cmd) free(last_cmd);
    return 0;
}

void handle_exit(char *buffer) {
    char copy[MAX_CMD_BUFFER];
    strcpy(copy, buffer);

    //skip the exit word
    strtok(copy, " ");
    char *code_str = strtok(NULL, " ");

    if (!code_str) { //no exit code
        printf("Bye!\n");
        exit(0);
    } else {
        int code = atoi(code_str) % 256;
        if (code < 0) code += 256;
        
        printf("bye\n");
        exit(code);
    }
}

void handle_echo(char *buffer) {
    char *to_print = buffer + 5; // buffer here is an addr. +5 will skip 'echo '
    if (strlen(to_print) > 0) {
        printf("%s\n", to_print);
    } else {
        printf("\n");
    }
}

void handle_double_bang(char **last_cmd) {
    if (!*last_cmd || strlen(*last_cmd) == 0) {// when last_cmd is empty
        return;
    }

    printf("%s\n", *last_cmd);
    process_cmd(*last_cmd, last_cmd); // wrap up processing logics into a chunk, reuse in both main() & double_bang()
}

void process_cmd(char *command, char **last_cmd) {
    //printf("you said: %s\n", command);

    char cmd_copy[MAX_CMD_BUFFER];// make a cp for command
    strcpy(cmd_copy, command);
    //printf("copy is: %s\n", cmd_copy);

    char *token = strtok(cmd_copy, " ");// get first token from copy, so ori command field will stay the same
    if (!token) return;

    if (strcmp(token, "exit") == 0) {
        handle_exit(command);
    } else if (strcmp(token, "echo") == 0) {
        handle_echo(command);
    } else if (strcmp(command, "!!") == 0) {
        handle_double_bang(last_cmd);
    } else {
        printf("bad command!\n");
    }

    if (strcmp(command, "!!") != 0) { // update the last_cmd field, !! shouldnt come with any other char

        if (*last_cmd) free(*last_cmd); // prevent mem leak

        //printf("last command before update is %s\n", command);
        *last_cmd = strdup(command);
    }
}

int run_script(char *path) {
    char buffer[MAX_CMD_BUFFER];
    char *last_cmd = NULL;

    FILE *fp = fopen(path, "r");

    if (!fp){ // file open failed
        fprintf(stderr, "cannot open file %s: %s\n", path, strerror(errno));
        return 1; // mark the failing by exit 1
    }

    while(fgets(buffer, MAX_CMD_BUFFER, fp)) {
        printf("Read: %s\n", buffer);

        buffer[strcspn(buffer, "\n")] = '\0';
        if (buffer[0] == '\0') continue;

        process_cmd(buffer, &last_cmd);
    }

    fclose(fp);
    free(last_cmd);
    return 0;
}
