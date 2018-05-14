#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "darkness.h"

static struct actor_def* map_find_lowest_tick(struct map_def *map);

const struct tile_type tile_types[] = {
    { '#', CP_WHITE,  0, "wall",  TILE_OPAQUE | TILE_SOLID },
    { '.', CP_WHITE,  0, "floor", 0 },
    { '+', CP_YELLOW, 0, "door", TILE_OPAQUE },
    { '>', CP_YELLOW, 0, "stair down", 0 },
};

const char *size_names[SIZE_COUNT] = {
    "Tiny", "Small", "Average", "Large", "Huge", "Excessive"
};
const char *goal_names[GOAL_COUNT] = {
    "Explore", "Exterminate", "Activate", "Collect", "Boss"
};


const struct tile_type* tile_get_info(int tt) {
    if (tt < 0) return NULL;
    return &tile_types[tt];
}

void shift_point(int *x, int *y, int direction) {
    switch(direction) {
        case DIR_NORTH:     *y -= 1;    break;
        case DIR_EAST:      *x -= 1;    break;
        case DIR_SOUTH:     *y += 1;    break;
        case DIR_WEST:      *x += 1;    break;
        case DIR_NORTHEAST: *x -= 1;    *y -= 1;    break;
        case DIR_NORTHWEST: *x += 1;    *y -= 1;    break;
        case DIR_SOUTHEAST: *x -= 1;    *y += 1;    break;
        case DIR_SOUTHWEST: *x += 1;    *y += 1;    break;
    }
}

struct map_def* map_create(int width, int height) {
    struct map_def *map = malloc(sizeof(struct map_def));
    if (!map) {
        return NULL;
    }
    map->width = width;
    map->height = height;

    map->tiles = calloc(width * height * sizeof(int), 1);
    if (!map->tiles) {
        free(map);
        return NULL;
    }

    map->visibility = calloc(width * height * sizeof(int), 1);
    if (!map->visibility) {
        free(map->tiles);
        free(map);
        return NULL;
    }

    map->actors = calloc(width * height * sizeof(struct actor_def*), 1);
    if (!map->actors) {
        free(map->tiles);
        free(map->visibility);
        free(map);
        return NULL;
    }

    map->rooms = NULL;
    map->log = NULL;
    map->turn_number = 0;
    return map;
}

void map_destroy(struct map_def *map) {
    message_freeall(map);

    struct room_def *room = map->rooms;
    while (room) {
        struct room_def *next = room->next;
        free(room);
        room = next;
    }

    const int map_tiles = map->width * map->height;
    for (int i = 0; i < map_tiles; ++i) {
        actor_destroy(map->actors[i]);
    }
    free(map->actors);
    free(map->visibility);
    free(map->tiles);
    free(map);
}

int map_valid_coord(const struct map_def *map, int x, int y) {
    if (x < 0 || y < 0 || x >= map->width || y >= map->height) {
        return 0;
    }
    return 1;
}
int map_get_tile(const struct map_def *map, int x, int y) {
    return map->tiles[x + y * map->width];
}

void map_set_tile(const struct map_def *map, int x, int y, int new_tile) {
    if (!map_valid_coord(map, x, y)) {
        return;
    }
    map->tiles[x + y * map->width] = new_tile;
}

struct actor_def* map_get_actor(const struct map_def *map, int x, int y) {
    return map->actors[x + y * map->width];
}

int map_set_actor(struct map_def *map, int x, int y, struct actor_def *actor) {
    if (!map_valid_coord(map, x, y)) {
        return 0;
    }
    if (map_get_actor(map, x, y) != NULL && actor != NULL) {
        return 0;
    }
    map->actors[x + y * map->width] = actor;
    if (actor) {
        actor->x = x;
        actor->y = y;
    }
    return 1;
}

int map_shift_actor(struct map_def *map, struct actor_def *actor, int direction) {
    int x = actor->x;
    int y = actor->y;
    shift_point(&x, &y, direction);

    if (!map_valid_coord(map, x, y)) {
        return 0;
    }
    if (map->actors[x + y * map->width]) {
        return 0;
    }

    map->actors[actor->x + actor->y * map->width] = NULL;
    actor->x = x;
    actor->y = y;
    map->actors[actor->x + actor->y * map->width] = actor;
    return 1;
}

int map_move_actor(struct map_def *map, struct actor_def *actor, int new_x, int new_y) {
    if (!map_valid_coord(map, new_x, new_y)) {
        return 0;
    }
    if (map->actors[new_x + new_y * map->width]) {
        return 0;
    }

    map->actors[actor->x + actor->y * map->width] = NULL;
    actor->x = new_x;
    actor->y = new_y;
    map->actors[actor->x + actor->y * map->width] = actor;
    return 1;
}

void map_find_space(const struct map_def *map, int *x, int *y) {
    int sx, sy;
    const struct tile_type *tile = NULL;
    do {
        sx = rand() % map->width;
        sy = rand() % map->height;
        tile = tile_get_info(map_get_tile(map, sx, sy));
    } while (!tile || tile->flags & TILE_SOLID || map_get_actor(map, sx, sy) != NULL);
    *x = sx;
    *y = sy;
}


static struct actor_def* map_find_lowest_tick(struct map_def *map) {
    const int map_size = map->width * map->height;
    struct actor_def *lowest = NULL;

    for (int i = 0; i < map_size; ++i) {
        struct actor_def *here = map->actors[i];
        if (here == NULL)   continue;

        if (lowest == NULL) {
            lowest = here;
            continue;
        }

        if (here->tick < lowest->tick) {
            lowest = here;
        }
    }

    return lowest;
}

void map_tick(struct map_def *map, struct actor_def *player) {
    ++map->turn_number;

    while (1) {
        struct actor_def *current = map_find_lowest_tick(map);
        if (current == player) return;
        actor_generic_ai(map, current);
    }
}

int map_was_seen(const struct map_def *map, int x, int y) {
    if (!map_valid_coord(map, x, y)) {
        return 0;
    }
    return (map->visibility[x + y * map->width] & MAP_WAS_SEEN) == MAP_WAS_SEEN;
}

int map_in_view(const struct map_def *map, int x, int y) {
    if (!map_valid_coord(map, x, y)) {
        return 0;
    }
    return (map->visibility[x + y * map->width] & MAP_IN_VIEW) == MAP_IN_VIEW;
}

static void map_fov_helper(struct map_def *map, int x, int y, int force_non_opaque) {
    if (!map_valid_coord(map, x, y)) {
        return;
    }
    if (map_in_view(map, x, y)) {
        return;
    }
    map->visibility[x + y * map->width] |= MAP_WAS_SEEN | MAP_IN_VIEW;

    int tile_no = map_get_tile(map, x, y);
    const struct tile_type *tile = tile_get_info(tile_no);
    if (!force_non_opaque && (!tile || tile->flags & TILE_OPAQUE)) {
        return;
    }

    map_fov_helper(map, x + 1, y + 1, 0);
    map_fov_helper(map, x + 1, y,     0);
    map_fov_helper(map, x + 1, y - 1, 0);
    map_fov_helper(map, x,     y + 1, 0);
    map_fov_helper(map, x,     y - 1, 0);
    map_fov_helper(map, x - 1, y + 1, 0);
    map_fov_helper(map, x - 1, y,     0);
    map_fov_helper(map, x - 1, y - 1, 0);
}

void map_do_fov(struct map_def *map, int from_x, int from_y, int range) {
    for (int y = 0; y < map->height; ++y) {
        for (int x = 0; x < map->width; ++x) {
            map->visibility[x + y * map->width] &= ~MAP_IN_VIEW;
        }
    }

    map_fov_helper(map, from_x, from_y, 1);
}

void map_dump(const struct map_def *map, const char *filename) {
    FILE *out = fopen(filename, "wt");
    if (!out) return;

    for (int y = 0; y < map->height; ++y) {
        for (int x = 0; x < map->width; ++x) {
            const struct tile_type *tile = tile_get_info(map_get_tile(map, x, y));
            const struct actor_def *actor = map_get_actor(map, x, y);

            if (map_in_view(map, x, y)) {
                if (actor) {
                    fputc(actor->my_class->glyph, out);
                    continue;
                }
            }
            fputc(tile->glyph, out);
        }
        fputc('\n', out);
    }

    fclose(out);
}

struct room_def* map_get_room(struct map_def *map, int x, int y) {
    struct room_def *current = map->rooms;
    while (current) {
        if (x > current->x1 && x < current->x2 && y > current->y1 && y < current->y2) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}