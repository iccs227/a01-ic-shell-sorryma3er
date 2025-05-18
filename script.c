#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "script.h"
#include "command.h"

#define MAX_CMD_BUFFER 255 // maximum length of input; final

int run_script(const char *path) {
    char buffer[MAX_CMD_BUFFER];
    char *last_cmd = NULL;

    FILE *fp = fopen(path, "r");

    if (!fp) {
        fprintf(stderr, "ERROR: cannot open file %s: %s\n", path, strerror(errno));
        return 1; // to match with the exit code in range of [0, 255]
    }

    while(fgets(buffer, MAX_CMD_BUFFER, fp)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (buffer[0] == '\0') continue; // skip when the line is empty

        process_cmd(buffer, &last_cmd, 0); // mode_indicator: 0 => script mode
    }

    fclose(fp);
    free(last_cmd);
    return 0;
}