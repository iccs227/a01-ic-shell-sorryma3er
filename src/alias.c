#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alias.h"

Alias *alias_head = NULL;

Alias *find_alias(const char *alias_name) {
    Alias *current = alias_head;
    while (current) {
        if (strcmp(current->alias_name, alias_name) == 0) {
            return current; // found the alias
        }
        current = current->next;
    }
    return NULL; // alias not found
}

void add_or_update_alias(const char *alias_name, const char *alias_val) {
    Alias *exist = find_alias(alias_name);

    if (exist) { // if exists, update the val
        free(exist->alias_val);
        exist->alias_val = strdup(alias_val);
        return;
    }

    // not exist, create a new alias
    Alias *new_node = malloc(sizeof(Alias));
    if (!new_node) {
        perror("Failed to allocate memory for new alias");
        exit(1);
    }

    new_node->alias_name = strdup(alias_name);
    new_node->alias_val = strdup(alias_val);
    new_node->next = NULL;

    if (alias_head == NULL) {
        alias_head = new_node;
    } else {
        // append at the tail
        Alias *curr = alias_head;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = new_node;
    }
}

int remove_alias(const char *alias_name) {
    Alias *curr = alias_head, *prev = NULL;

    while (curr) {
        if (strcmp(curr->alias_name, alias_name) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                alias_head = curr->next;
            }

            free(curr->alias_name);
            free(curr->alias_val);
            free(curr);
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    return -1;
}

void list_aliases(void) {
    Alias *curr = alias_head;
    int count = 0;
    while (curr) {
        printf("alias [%i]: %s='%s'\n", ++count, curr->alias_name, curr->alias_val);
        curr = curr->next;
    }
}