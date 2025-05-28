#include <ctype.h>
#include <string.h>

// remove trailing spaces/tabs
void rtrim(char *s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n-1])) {
        s[--n] = '\0';
    }
}

// remove leading spaces/tabs
void ltrim(char *s) {
    size_t i = 0, n = strlen(s);
    while (i < n && isspace((unsigned char)s[i])) {
        i++;
    }
    if (i > 0) {
        memmove(s, s + i, n - i + 1);  // include the null
    }
}

void trim(char *s) {
    rtrim(s);
    ltrim(s);
}