#include <stdlib.h>
#include <string.h>

#include "darkness.h"

#include <ncurses.h>

const int map_sizes[SIZE_COUNT] = {
    16, 32, 48, 64, 80, 96
};

void draw_map(struct map_def *map);

static struct room_def* random_room(struct room_def *first, int room_count);
static int point_direction(struct map_def *map, int x, int y);
static void point_on_border(struct map_def *map, struct room_def *room, int *x, int *y);
static void point_in_room(struct map_def *map, struct room_def *room, int *x_out, int *y_out);
static int map_area_is(struct map_def *map, int x1, int y1, int x2, int y2, int tile);
static int validate_room(struct map_def *map, struct room_def *room);
static struct room_def* make_room(struct map_def *map, int x, int y);

static struct room_def* random_room(struct room_def *first, int room_count) {
    if (first == NULL || room_count == 0) {
        return NULL;
    }

    int room_no = rng_max(room_count);
    int i = 0;
    struct room_def *result = first;
    while (result && i < room_no) {
        result = result->next;
        ++i;
    }
    return result;
}

static int point_direction(struct map_def *map, int x, int y) {
    int dir = DIR_NONE;
    int count = 0;
    for (int i = 0; i < DIR_NONE; i += 2) {
        int nx = x;
        int ny = y;
        shift_point(&nx, &ny, i);
        if (map_get_tile(map, nx, ny) == 1) {
            ++count;
            dir = i;
        }
    }
    if (count != 1) {
        return DIR_NONE;
    } else {
        return dir;
    }
}

static void point_on_border(struct map_def *map, struct room_def *room, int *x, int *y) {
    switch (rng_max(4)) {
        case 0: // north
            *x = room->x1 + 1 + rng_max(room->x2 - room->x1 - 2);
            *y = room->y1;
            break;
        case 1: // east
            *x = room->x2;
            *y = room->y1 + 1 + rng_max(room->y2 - room->y1 - 2);
            break;
        case 2: // south
            *x = room->x1 + 1 + rng_max(room->x2 - room->x1 - 2);
            *y = room->y2;
            break;
        case 3: // west
            *x = room->x1;
            *y = room->y1 + 1 + rng_max(room->y2 - room->y1 - 2);
            break;
    }
}

static void point_in_room(struct map_def *map, struct room_def *room, int *x_out, int *y_out) {
    int width = room->x2 - room->x1 - 2;
    int height = room->y2 - room->y1 - 2;
    int iterations = 0;
    while (iterations < 100) {
        ++iterations;
        int x = room->x1 + 1 + rand() % width;
        int y = room->y1 + 1 + rand() % height;
        if (map_get_tile(map, x, y) == 1 && map_get_actor(map, x, y) == NULL) {
            *x_out = x;
            *y_out = y;
            return;
        }
    }
    *x_out = -1;
}

static int map_area_is(struct map_def *map, int x1, int y1, int x2, int y2, int tile) {
    for (int y = y1; y <= y2; ++y) {
        for (int x = x1; x <= x2; ++x) {
            if (map_get_tile(map, x, y) != tile) {
                return 0;
            }
        }
    }
    return 1;
}

static int validate_room(struct map_def *map, struct room_def *room) {
    if (room->x1 < 0 || room->y1 < 0 || room->x2 >= map->width || room->y2 >= map->height) {
        return 0;
    }

    if (!map_area_is(map, room->x1, room->y1, room->x2, room->y2, 0)) {
        return 0;
    }

    return 1;
}

static struct room_def* make_room(struct map_def *map, int x, int y) {
    struct room_def *room = malloc(sizeof(struct room_def));
    if (!room) {
        return NULL;
    }

    int width = 4 + rng_max(5);
    int height = 4 + rng_max(5);

    int dir = point_direction(map, x, y);
    switch(dir) {
        case DIR_NORTH:
            x -= 1 + rng_max(width - 1);
            break;
        case DIR_WEST:
            x -= width;
            y -= 1 + rng_max(height - 1);
            break;
        case DIR_SOUTH:
            y -= height;
            x -= 1 + rng_max(width - 1);
            break;
        case DIR_EAST:
            y -= 1 + rng_max(height - 1);
            break;
    }

    room->explored = 0;
    room->type = -1;
    room->next = NULL;
    room->x1 = x;
    room->y1 = y;
    room->x2 = x + width;
    room->y2 = y + height;

    if (!validate_room(map, room)) {
        free(room);
        return NULL;
    }

    for (int cy = room->y1 + 1; cy <= room->y2 - 1; ++cy) {
        for (int cx = room->x1 + 1; cx <= room->x2 - 1; ++cx) {
            map_set_tile(map, cx, cy, 1);
        }
    }
    return room;
}

struct map_def* map_generate(struct dungeon_def *dungeon) {
    const int max_failures = 1000;
    int failure_count = 0;
    int room_count = 1;

    struct map_def *map = map_create(
        map_sizes[dungeon->size] + rng_max(16),
        map_sizes[dungeon->size] + rng_max(16));
    memset(map->tiles, 0, sizeof(map->width * map->height * sizeof(int)));

    struct room_def *rooms = NULL;
    do {
        int x, y;
        x = rng_max(map->width);
        y = rng_max(map->height);
        rooms = make_room(map, x, y);
    } while (rooms == NULL);

    while (failure_count < max_failures) {
        struct room_def *room, *parent;

        int x, y;
        parent = random_room(rooms, room_count);
        if (!parent) {
            ++failure_count;
            continue;
        }
        point_on_border(map, parent, &x, &y);

        room = make_room(map, x, y);
        if (room) {
            map_set_tile(map, x, y, 2);
            room->next = rooms;
            rooms = room;
            ++room_count;
        } else {
            ++failure_count;
        }
    }

    struct room_def *room;
    room = random_room(rooms, room_count);
    if (room) {
        int x = 1 + room->x1 + rng_max(room->x2 - room->x1 - 1);
        int y = 1 + room->y1 + rng_max(room->y2 - room->y1 - 1);
        map_set_tile(map, x, y, 3);
    }

    room = rooms;
    while (room) {
        int roll = rng_max(100);
        if (roll < 60) {
            // monster room
            int type = 100 + rng_max(2);
            int count = 2 + rng_max(3);
            for (int i = 0; i < count; ++i) {
                int x, y;
                point_in_room(map, room, &x, &y);
                if (x == -1) continue;


                struct actor_def *new_actor = actor_new(type);
                if (!new_actor) continue;
                new_actor->side = 1;
                new_actor->tick = 1;
                map_set_actor(map, x, y, new_actor);
            }
        }

        room = room->next;
    }

    map->rooms = rooms;
    return map;
}
