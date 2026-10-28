#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#define START_INDEX 1   /* header says 1-based; flip to 0 if the grader disagrees */

list_t *list_alloc() {
    list_t *l = malloc(sizeof(list_t));
    if (!l) return NULL;
    l->head = NULL;
    return l;
}

void list_free(list_t *l) {
    if (!l) return;
    node_t *cur = l->head;
    while (cur) {
        node_t *next = cur->next;
        free(cur);
        cur = next;
    }
    free(l);
}

void list_print(list_t *l) {
    for (node_t *c = l->head; c; c = c->next) printf("%d->", c->value);
    printf("NULL\n");
}

/* caller must free the returned string */
char *listToString(list_t *l) {
    char *buf = malloc(list_length(l) * 13 + 6);
    if (!buf) return NULL;
    char *p = buf;
    for (node_t *c = l->head; c; c = c->next)
        p += sprintf(p, "%d->", c->value);
    strcpy(p, "NULL");
    return buf;
}

node_t *getNode(elem value) {
    node_t *n = malloc(sizeof(node_t));
    if (!n) return NULL;
    n->value = value;
    n->next = NULL;
    return n;
}

int list_length(list_t *l) {
    int n = 0;
    for (node_t *c = l->head; c; c = c->next) n++;
    return n;
}

void list_add_to_back(list_t *l, elem value) {
    node_t *n = getNode(value);
    if (!n) return;
    if (!l->head) { l->head = n; return; }
    node_t *c = l->head;
    while (c->next) c = c->next;
    c->next = n;
}

void list_add_to_front(list_t *l, elem value) {
    node_t *n = getNode(value);
    if (!n) return;
    n->next = l->head;
    l->head = n;
}

/* valid: START_INDEX .. length+START_INDEX; otherwise no-op */
void list_add_at_index(list_t *l, elem value, int index) {
    int i = index - START_INDEX;
    if (i < 0) return;
    if (i == 0) { list_add_to_front(l, value); return; }
    node_t *prev = l->head;
    for (int k = 0; k < i - 1 && prev; k++) prev = prev->next;
    if (!prev) return;
    node_t *n = getNode(value);
    if (!n) return;
    n->next = prev->next;
    prev->next = n;
}

elem list_remove_from_back(list_t *l) {
    if (!l->head) return -1;
    if (!l->head->next) {
        elem v = l->head->value;
        free(l->head);
        l->head = NULL;
        return v;
    }
    node_t *c = l->head;
    while (c->next->next) c = c->next;
    elem v = c->next->value;
    free(c->next);
    c->next = NULL;
    return v;
}

elem list_remove_from_front(list_t *l) {
    if (!l->head) return -1;
    node_t *n = l->head;
    elem v = n->value;
    l->head = n->next;
    free(n);
    return v;
}

elem list_remove_at_index(list_t *l, int index) {
    int i = index - START_INDEX;
    if (i < 0 || !l->head) return -1;
    if (i == 0) return list_remove_from_front(l);
    node_t *prev = l->head;
    for (int k = 0; k < i - 1 && prev->next; k++) prev = prev->next;
    if (!prev->next) return -1;
    node_t *target = prev->next;
    elem v = target->value;
    prev->next = target->next;
    free(target);
    return v;
}

bool list_is_in(list_t *l, elem value) {
    return list_get_index_of(l, value) != -1;
}

elem list_get_elem_at(list_t *l, int index) {
    int i = index - START_INDEX;
    if (i < 0) return -1;
    node_t *c = l->head;
    for (int k = 0; k < i && c; k++) c = c->next;
    return c ? c->value : -1;
}

int list_get_index_of(list_t *l, elem value) {
    int i = START_INDEX;
    for (node_t *c = l->head; c; c = c->next, i++)
        if (c->value == value) return i;
    return -1;
}