/* ICCS227: Project 1: icsh
 * Name: Haicheng Wang
 * StudentID: 6580244
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define MAX_CMD_BUFFER 255 // maximum length of input; final
#define MAX_ARGS 16 // assume the external command contains no more than 16 words; final

//refactor out by features to make the main() cleaner
void handle_exit(char *buffer, int mode_indicator);
void handle_echo(char *buffer);
void handle_double_bang(char **last_cmd, int mode_indicator);
void process_cmd(char *command, char **last_cmd, int mode_indicator);
void split_args(char *to_split, char *argv[]);
void run_external(char *argv[]);
int run_script(char *path);

int main(int argc, char **argv) { //argc for count, argv is array of argument
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

void handle_exit(char *buffer, int mode_indicator) {
    char copy[MAX_CMD_BUFFER];
    strcpy(copy, buffer);

    //skip the exit word
    strtok(copy, " ");
    char *code_str = strtok(NULL, " ");

    if (!code_str) { //no exit code
        if (mode_indicator) printf("bye\n"); // print bye only for interactive
        exit(0);
    } else {
        int code = atoi(code_str) % 256;
        if (code < 0) code += 256;
        
        if (mode_indicator) printf("bye\n");
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

void handle_double_bang(char **last_cmd, int mode_indicator) {
    if (!*last_cmd || strlen(*last_cmd) == 0) {// when last_cmd is empty
        return;
    }

    if (mode_indicator) printf("%s\n", *last_cmd); // reprint only for interactive mode
    process_cmd(*last_cmd, last_cmd, mode_indicator); // wrap up processing logics into a chunk, reuse in both main() & double_bang()
}

void process_cmd(char *command, char **last_cmd, int mode_indicator) {
    //printf("you said: %s\n", command);

    char cmd_copy[MAX_CMD_BUFFER];// make a cp for command
    strcpy(cmd_copy, command);
    //printf("copy is: %s\n", cmd_copy);

    char *token = strtok(cmd_copy, " ");// get first token from copy, so ori command field will stay the same
    if (!token) return;

    if (strcmp(token, "exit") == 0) {
        handle_exit(command, mode_indicator);
    } else if (strcmp(token, "echo") == 0) {
        handle_echo(command);
    } else if (strcmp(command, "!!") == 0) {
        handle_double_bang(last_cmd, mode_indicator);
    } else {
        /*External command handle: */
        char *argv[MAX_ARGS + 1]; // +1 for NULL sentinel
 
        char to_split[MAX_CMD_BUFFER]; // since cmd_copy is alr tokenized and contains \0 inside
        strcpy(to_split, command);

        split_args(to_split, argv); // argv[i] points to something that initialize in this scope, prevent dangling pointer

        if (argv[0]) run_external(argv);
    }

    if (strcmp(command, "!!") != 0) { // update the last_cmd field, !! shouldnt come with any other char

        if (*last_cmd) free(*last_cmd); // prevent mem leak

        //printf("last command before update is %s\n", command);
        *last_cmd = strdup(command);
    }
}

void run_external(char *argv[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed: ");
        return;
    } else if (pid == 0) { // child process
        execvp(argv[0], argv);

        fprintf(stderr, "running external command failed: %s, %s\n", argv[0], strerror(errno)); // reach here only on failure
        exit(1);
    } else { // parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

void split_args(char *to_split, char *argv[]) {
    int ind = 0;    
    char *token = strtok(to_split, " ");
    while(token && ind < MAX_ARGS) {
        argv[ind] = token;
        token = strtok(NULL, " ");
        ind ++;
    }
    argv[ind] = NULL; // add NULL sentinel
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
        //printf("Read: %s", buffer);

        buffer[strcspn(buffer, "\n")] = '\0';
        if (buffer[0] == '\0') continue; // skip when the line is empty

        process_cmd(buffer, &last_cmd, 0); // mode_indicator: 0 => script mode
    }

    fclose(fp);
    free(last_cmd);
    return 0;
}
