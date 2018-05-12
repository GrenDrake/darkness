#include <stdio.h>
#include <stdlib.h>

#include "darkness.h"

// int ident;
// const char *name;
// const struct attack_def *attacks[MAX_CLASS_ATTACKS];
// int base_stats[STAT_COUNT];
// const char *description;
// STATS:  HP ACC DODG DAM_MIN DAM_MAX PROT SPD CRIT

const struct class_def classes[] = {
    { 0,   "soldier", '@', { 0 }, { 7, 50, 0, 2, 4, 0, 5, 10 }, "A warrior trained in the service of armies." },
    { 1,   "medic",   '@', { 0 }, { 5, 50, 0, 2, 4, 0, 5, 10 }, "A battlefield medic, trained during the last war." },
    { 100, "goblin",  'g', { 0 }, { 3, 50, 0, 2, 4, 0, 5, 10 }, "A warrior trained in the service of armies." },
    { 101, "orc",     'o', { 0 }, { 3, 50, 0, 2, 4, 0, 5, 10 }, "A warrior trained in the service of armies." },
    { -1, NULL },
};

const struct class_def* class_get(int ident) {
    int i = 0;
    while (classes[i].name != NULL) {
        if (classes[i].ident == ident) {
            return &classes[i];
        }
        ++i;
    }
    return NULL;
}

void actor_generic_ai(struct map_def *map, struct actor_def *actor) {
    actor_action_step(map, actor, rand() % DIR_COUNT);
    actor->tick += 3;
}


struct actor_def* actor_new(int class_ident) {
    struct actor_def *actor = calloc(1, sizeof(struct actor_def));
    if (!actor) return NULL;

    actor->class_ident = class_ident;
    actor->my_class = class_get(class_ident);
    if (!actor->my_class) {
        free(actor);
        return NULL;
    }

    actor->hp = actor->my_class->base_stats[STAT_MAXHP];

    return actor;
}

int actor_action_step(struct map_def *map, struct actor_def *actor, int direction) {
    int nx = actor->x;
    int ny = actor->y;
    shift_point(&nx, &ny, direction);

    const struct tile_type *tile = tile_get_info(map_get_tile(map, nx, ny));
    if (!tile || tile->flags & TILE_SOLID) {
        return 0;
    }

    if (map_get_actor(map, nx, ny) != NULL) {
        return 0;
    }

    map_shift_actor(map, actor, direction);
    actor->tick += TICK_MOVE;
    return 1;
}

int actor_get_stat(const struct actor_def *actor, int stat_number) {
    if (actor == NULL)  return -1;

    if (stat_number == STAT_HP) {
        return actor->hp;
    } else {
        return actor->my_class->base_stats[stat_number];
    }
}
