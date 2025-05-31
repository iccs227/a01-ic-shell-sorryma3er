#ifndef ALIAS_H
#define ALIAS_H

#include <stdbool.h>

typedef struct Alias{
    char *alias_name; // the nickname
    char *alias_val; // the real command
    struct Alias *next;
} Alias;

extern Alias *alias_head; // head of the alias linked list

Alias *find_alias(const char *alias_name); // find an alias by its name

void add_or_update_alias(const char *alias_name, const char *alias_val); // add a new alias or update an existing one

int remove_alias(const char *alias_name); // remove an alias by its name, returns -1 if not found

void list_aliases(void); // list all aliases

#endif