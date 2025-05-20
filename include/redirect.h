#ifndef REDIRECT_H
#define REDIRECT_H

typedef struct {
    char *inputFile;
    char *outputFile;
} Redirect;

int parse_redirect(char *argv[], Redirect *redirect); // parse the command line arguments and set up the redirect structure

int apply_redirect(Redirect *redirect); // apply the redirection by modifying the file descriptors

void free_redirect(Redirect *redirect); // free the memory allocated for the redirect structure

#define PANIC(args...) fprintf(stderr, args)

#endif