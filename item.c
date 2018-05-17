#include <stdlib.h>

#include "darkness.h"

const struct item_type item_types[] = {
    {   1,  "test",   1 },
    {   2,  "apple",  3 },
    {   3,  "turnip", 6 },
    {   -1, NULL }
};

const struct item_type* itemtype_get(int ident) {
    int i = 0;
    while (item_types[i].name != NULL) {
        if (item_types[i].ident == ident) {
            return &item_types[i];
        }
        ++i;
    }
    return NULL;
}

struct item_def* item_new(int type_ident, int qty) {
    struct item_def *item = calloc(1, sizeof(struct item_def));
    if (!item) return NULL;

    item->qty = qty;
    item->type_id = type_ident;
    item->my_type = itemtype_get(type_ident);
    if (!item->my_type) {
        free(item);
        return NULL;
    }

    return item;
}

void item_destroy(struct item_def *item) {
    free(item);
}
