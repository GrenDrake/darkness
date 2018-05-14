#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "darkness.h"

void message_add(struct map_def *map, const char *text) {
    struct message_def *msg = malloc(sizeof(struct message_def));
    if (!msg) return;

    msg->msg = malloc(strlen(text) + 1);
    if (!msg->msg) {
        free(msg);
        return;
    }

    strcpy(msg->msg, text);
    msg->msg[0] = toupper(msg->msg[0]);
    msg->next = map->log;
    msg->turn_number = map->turn_number;
    map->log = msg;
}

void message_format(struct map_def *map, const char *text, ...) {
    struct message_def *msg = malloc(sizeof(struct message_def));
    if (!msg) return;

    va_list args, args_copy;
    va_start(args, text);

    va_copy(args_copy, args);
    size_t length = vsnprintf(NULL, 0, text, args_copy);
    va_end(args_copy);

    msg->msg = malloc(length + 1);
    if (!msg->msg) {
        va_end(args);
        free(msg);
        return;
    }

    vsprintf(msg->msg, text, args);
    msg->next = map->log;
    map->log = msg;
    msg->turn_number = map->turn_number;
    msg->msg[0] = toupper(msg->msg[0]);
    va_end(args);
}

void message_freeall(struct map_def *map) {
    struct message_def *cur = map->log, *next;
    while (cur) {
        next = cur->next;
        free(cur->msg);
        free(cur);
        cur = next;
    }
}
