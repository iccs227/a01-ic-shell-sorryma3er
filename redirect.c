#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "redirect.h"

int parse_redirect(char *argv[], Redirect *redirect) { // leave the real external command inside of argv[], and assign value to *redirect
    // initialize the redirect type
    redirect->inputFile = NULL;
    redirect->outputFile = NULL;

    int read_idx = 0, write_idx = 0;
    while (argv[read_idx] != NULL) {
        if (strcmp(argv[read_idx], "<") == 0) {
            read_idx ++; // skip the '<' token
            if (argv[read_idx] == NULL) { // no input file specified
                PANIC("ERROR: No input file specified\n");
                return -1;
            } else {
                redirect->inputFile = strdup(argv[read_idx]);
                read_idx ++; // skip the input file name
            }
        } else if (strcmp(argv[read_idx], ">") == 0) {
            read_idx ++; // skip the '>' token
            if (argv[read_idx] == NULL) { 
                PANIC("ERROR: No output file specified\n");
                return -1;
            } else {
                redirect->outputFile = strdup(argv[read_idx]);
                read_idx ++; // skip the output file name
            }
        } else { // its a real command
            argv[write_idx] = argv[read_idx]; // copy the command
            write_idx ++;
            read_idx ++;
        }
    }
    argv[write_idx] = NULL; // add NULL sentinel for execvp()
    return 0;
}

int apply_redirect(Redirect *redirect) {
    if (redirect->inputFile != NULL) { // for '<', which redirect STDIN to what inputFileDecr points to, 
        int inputFileDescr;
        int inputOpenFlags = O_RDONLY;

        inputFileDescr = open(redirect->inputFile, inputOpenFlags);
        if (inputFileDescr < 0) {
            PANIC("ERROR: Cannot open input file %s: %s\n", redirect->inputFile, strerror(errno));
            return -1;
        }

        int result = dup2(inputFileDescr, 0); // 0 -> STDIN_FILENO
        if (result < 0) {
            PANIC("ERROR: Cannot redirect input to file %s: %s\n", redirect->inputFile, strerror(errno));
            close(inputFileDescr);
            return -1;
        }
        close(inputFileDescr);
    }
    if (redirect->outputFile != NULL) { // for '>', redirect STDOUT to what outFileDescr points to
        int outFileDescr;
        int outFileFlags = O_TRUNC | O_CREAT | O_WRONLY;
        int outPermissions = 0666;

        outFileDescr = open(redirect->outputFile, outFileFlags, outPermissions);
        if (outFileDescr < 0) {
            PANIC("ERROR: Cannot open output file %s: %s\n", redirect->outputFile, strerror(errno));
            return -1;
        }

        int result = dup2(outFileDescr, 1); // 1 -> STDOUT_FILENO
        if (result < 0) {
            PANIC("ERROR: Cannot redirect output to file %s: %s\n", redirect->outputFile, strerror(errno));
            close(outFileDescr);
            return -1;
        }
        close(outFileDescr);
    }
    return 0;
}

void free_redirect(Redirect *redirect) {
    free(redirect->inputFile);
    free(redirect->outputFile);
    redirect->inputFile = NULL;
    redirect->outputFile = NULL;
}