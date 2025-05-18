#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "exec.h"

#define MAX_ARGS 16 // assume the external command contains no more than 16 words; final

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

void split_args(char *to_split, char *argv[]){
    int idx = 0;
    char *token = strtok(to_split, " "); // get the first word
    while(token && idx < MAX_ARGS) {
        argv[idx] = token; // store the token in the array
        token = strtok(NULL, " "); // get the next word
        idx++;
    }
    argv[idx] = NULL; // add NULL sentinel
}