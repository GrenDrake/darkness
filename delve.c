#include <stdio.h>
#include <stdlib.h>

#include <ncurses.h>

#include "darkness.h"

void draw_map(struct map_def *map, int cx, int cy);
void player_action(struct map_def *map, struct actor_def *player, int action);
void delve_loop(struct dungeon_def *dungeon);

const int max_messages = 10;
const int status_width = 40;
static int max_x = 0;
static int max_y = 0;

void draw_map(struct map_def *map, int cx, int cy) {
    int map_half_height = max_y / 2;
    int map_half_width = (max_x - status_width)/ 2;

    for (int y = cy - map_half_height, sy = 0; y < cy + map_half_height; ++y, ++sy) {
        for (int x = cx - map_half_width, sx = 0; x < cx + map_half_width; ++x, ++sx) {
            if (!map_valid_coord(map, x, y))    continue;
            if (!map_was_seen(map, x, y))       continue;

            const struct tile_type *tile = tile_get_info(map_get_tile(map, x, y));
            const struct actor_def *actor = map_get_actor(map, x, y);

            if (map_in_view(map, x, y)) {
                attrset(A_BOLD);
                if (actor) {
                    attron(COLOR_PAIR(actor->my_class->color));
                    mvaddch(sy, sx, actor->my_class->glyph);
                    continue;
                }
            } else {
                attrset(0);
            }
            attron(COLOR_PAIR(tile->color));
            mvaddch(sy, sx, tile->glyph);
        }
    }
    attrset(0);
}

void draw_log(struct map_def *map) {
    struct message_def *msg = map->log;
    int count = 0;

    while (count < max_messages && msg) {
        if (msg->turn_number == map->turn_number - 1) {
            attrset(A_BOLD | COLOR_PAIR(CP_GREEN));
        }
        mvprintw(max_y - count - 1, max_x - status_width + 1, "%s", msg->msg);
        attrset(0);

        ++count;
        msg = msg->next;
    }
}

void player_action(struct map_def *map, struct actor_def *player, int action) {
    int success = 0;
    switch(action) {
        case ACTION_BASIC_ATTACK:
            break;

        case ACTION_WAIT:
            player->tick += TICK_MOVE + 3;
            success = 1;
            break;
        case ACTION_STEP_NORTH:
        case ACTION_STEP_EAST:
        case ACTION_STEP_SOUTH:
        case ACTION_STEP_WEST:
        case ACTION_STEP_NORTHWEST:
        case ACTION_STEP_SOUTHWEST:
        case ACTION_STEP_NORTHEAST:
        case ACTION_STEP_SOUTHEAST:
            success = actor_action_step(map, player, action);
            if (!success) {
                int x = player->x;
                int y = player->y;
                shift_point(&x, &y, action);
                struct actor_def *who = map_get_actor(map, x, y);
                if (who) {
                    actor_do_attack(map, player, who, NULL);
                    player->tick += TICK_MOVE;
                    success = 1;
                }
            }
            break;
    }

    if (success) {
        map_do_fov(map, player->x, player->y, 4);
        map_tick(map, player);
    }
}

struct map_def* map_new() {
    int map_width = 40 * (1 + rng_max(4));
    int map_height = 15 * (1 + rng_max(4));
    struct map_def *map = map_create(map_width, map_height);
    map->log = NULL;
    return map;
}

void delve_loop(struct dungeon_def *dungeon) {
    struct map_def *map = map_generate(dungeon);
    if (!map) {
        printw("COULD NOT ALLOCATE MAP MEMORY");
        getch();
        return;
    }
    int px = 0, py = 0;
    map_find_space(map, &px, &py);
    map_set_actor(map, px, py, dungeon->player);
    dungeon->player->tick = 0;
    map_do_fov(map, dungeon->player->x, dungeon->player->y, 4);
    message_format(map, "Beginning mission with %s.", dungeon->player->name);
    ++map->turn_number;

    while (!wants_to_quit) {
        getmaxyx(stdscr, max_y, max_x);
        clear();

        for (int i = 0; i < max_y; ++i) {
            mvaddch(i, max_x - status_width, ACS_CKBOARD);
        }
        for (int i = 0; i < max_x / 2; ++i) {
            mvaddch(max_y - max_messages - 1, max_x - status_width + i, ACS_CKBOARD);
        }
        mvprintw(max_y - max_messages - 1, max_x - status_width + 2, " %s %s ",
            size_names[dungeon->size],
            goal_names[dungeon->goal]);

        draw_map(map, dungeon->player->x, dungeon->player->y);
        draw_log(map);
        refresh();

        int key = getch();

        switch (key) {
        // debug related
            case 'E': // end quest
                map_set_actor(map, dungeon->player->x, dungeon->player->y, NULL);
                map_destroy(map);
                return;
            case 'M': // reveal map
                for (int y = 0; y < map->height; ++y) {
                    for (int x = 0; x < map->width; ++x) {
                        map->visibility[x + y * map->width] |= MAP_WAS_SEEN | MAP_IN_VIEW;
                    }
                }
                break;
            case 'L':
                message_add(map, "Message test.");
                break;
            case 'D':
                map_dump(map, "map.txt");
                break;

        // ability related
            case 'b':
                player_action(map, dungeon->player, ACTION_BASIC_ATTACK);
                break;

        //  movement related
            case ' ':
            case KEY_B2:
                player_action(map, dungeon->player, ACTION_WAIT);
                break;
            case KEY_HOME:
            case KEY_A1:
                player_action(map, dungeon->player, ACTION_STEP_NORTHEAST);
                break;
            case KEY_PPAGE:
            case KEY_A3:
                player_action(map, dungeon->player, ACTION_STEP_NORTHWEST);
                break;
            case KEY_END:
            case KEY_C1:
                player_action(map, dungeon->player, ACTION_STEP_SOUTHEAST);
                break;
            case KEY_NPAGE:
            case KEY_C3:
                player_action(map, dungeon->player, ACTION_STEP_SOUTHWEST);
                break;
            case KEY_LEFT:
                player_action(map, dungeon->player, ACTION_STEP_EAST);
                break;
            case KEY_RIGHT:
                player_action(map, dungeon->player, ACTION_STEP_WEST);
                break;
            case KEY_UP:
                player_action(map, dungeon->player, ACTION_STEP_NORTH);
                break;
            case KEY_DOWN:
                player_action(map, dungeon->player, ACTION_STEP_SOUTH);
                break;
            case 'Q':
                wants_to_quit = 1;
                map_set_actor(map, dungeon->player->x, dungeon->player->y, NULL);
                map_destroy(map);
                return;
        }
    }

    map_set_actor(map, dungeon->player->x, dungeon->player->y, NULL);
    map_destroy(map);
}
