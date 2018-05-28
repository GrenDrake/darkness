#include <stdio.h>
#include <stdlib.h>

#include "console.h"
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
                con_setattr(CON_BOLD);
                if (actor) {
                    con_setcolour(actor->my_class->color);
                    con_addchar(sx, sy, actor->my_class->glyph);
                    continue;
                }
            } else {
                con_resetattr(0);
            }
            con_setcolour(tile->color);
            con_addchar(sx, sy, tile->glyph);
        }
    }
    con_resetattr(0);
}

void draw_log(struct map_def *map) {
    struct message_def *msg = map->log;
    int count = 0;

    while (count < max_messages && msg) {
        if (msg->turn_number == map->turn_number - 1) {
            con_setattr(CON_BOLD);
            con_setcolour(CP_GREEN);
        }
        con_addstr(max_x - status_width + 1, max_y - count - 1, "%s", msg->msg);
        con_resetattr(0);

        ++count;
        msg = msg->next;
    }
}

static void draw_player_stats(struct actor_def *player) {
    int left_margin = max_x - status_width + 1;
    con_addstr(left_margin, 0, "%s the %s", player->name, player->my_class->name);
    con_addstr(left_margin, 2, "HP: %d/%d", player->hp, actor_get_stat(player, STAT_MAXHP));
    con_addstr(left_margin, 3, "ACC: %d", actor_get_stat(player, STAT_ACCURACY));
    con_addstr(left_margin  + status_width / 2, 3, "DODGE: %d", actor_get_stat(player, STAT_DODGE));
    con_addstr(left_margin, 4, "DAMAGE: %d-%d", actor_get_stat(player, STAT_DAMAGE_MIN), actor_get_stat(player, STAT_DAMAGE_MAX));
    con_addstr(left_margin  + status_width / 2, 4, "PROTECTION: %d", actor_get_stat(player, STAT_PROTECTION));
    con_addstr(left_margin, 5, "SPEED: %d", actor_get_stat(player, STAT_SPEED));
    con_addstr(left_margin  + status_width / 2, 5, "CRITICAL: %d%%", actor_get_stat(player, STAT_CRITICAL));
}

static void draw_player_inventory(struct actor_def *player) {
    int left_margin = max_x - status_width + 1;
    con_addstr(left_margin, 0, "%s the %s", player->name, player->my_class->name);
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        if (player->inventory[i] == NULL) {
            con_addstr(left_margin, 2 + i, "%d) (empty)", i + 1);
        } else {
            con_addstr(left_margin, 2 + i, "%d) %s (x%d)",
                i + 1,
                player->inventory[i]->my_type->name,
                player->inventory[i]->qty);
        }
    }
}


void test_dungeon_complete(struct dungeon_def *dungeon, struct map_def *map) {
    if (dungeon->complete) {
        return;
    }

    int total = 0, valid = 0;
    switch(dungeon->goal) {
        case GOAL_KILLALL:
            for (int y = 0; y < map->height; ++y) {
                for (int x = 0; x < map->width; ++x) {
                    struct actor_def *actor = map_get_actor(map, x, y);
                    if (actor && actor != dungeon->player) {
                        return;
                    }
                }
            }
            dungeon->complete = 1;
            return;
        case GOAL_EXPLORE: {
            struct room_def *room = map->rooms;
            while (room) {
                ++total;
                if (room->explored) {
                    ++valid;
                }
                room = room->next;
            }
            if (valid >= total * 80 / 100) {
                dungeon->complete = 1;
            }
            break;
        }
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
                    player->tick += TICK_ATTACK;
                    success = 1;
                }
            } else {
                struct room_def *room = map_get_room(map, player->x, player->y);
                if (room) {
                    room->explored = 1;
                }
            }
            break;
    }

    if (success) {
        map_do_fov(map, player->x, player->y, 4);
        map_tick(map, player);
    }
}


void delve_loop(struct dungeon_def *dungeon) {
    struct map_def *map = map_generate(dungeon);
    if (!map) {
        con_addstr(0, 0, "COULD NOT ALLOCATE MAP MEMORY");
        con_getc();
        return;
    }
    int px = 0, py = 0;
    map_find_space(map, &px, &py);
    map_set_actor(map, px, py, dungeon->player);
    dungeon->player->tick = 0;
    map_do_fov(map, dungeon->player->x, dungeon->player->y, 4);
    message_format(map, "Beginning mission with %s.", dungeon->player->name);
    ++map->turn_number;

    int display_mode = 0;
    int scroll_x, scroll_y;
    while (!wants_to_quit) {
        if (map->player->hp > 0) {
            scroll_x = map->player->x;
            scroll_y = map->player->y;
        }
        test_dungeon_complete(dungeon, map);

        max_x = con_maxx();
        max_y = con_maxy();
        con_clear();

        for (int i = 0; i < max_y; ++i) {
            con_addchar(max_x - status_width, i, con_special(CON_CHAR_CHECKER));
        }
        for (int i = 0; i < max_x / 2; ++i) {
            con_addchar(max_x - status_width + i, max_y - max_messages - 1, con_special(CON_CHAR_CHECKER));
        }
        if (dungeon->complete) {
            con_addstr(max_x - status_width + 2, max_y - max_messages - 1, " COMPLETE ");
        } else {
            con_addstr(max_x - status_width + 2, max_y - max_messages - 1, " %s %s ",
                size_names[dungeon->size],
                goal_names[dungeon->goal]);
        }

        draw_map(map, scroll_x, scroll_y);
        draw_log(map);
        switch(display_mode) {
            case 0: draw_player_stats(dungeon->player); break;
            case 1: draw_player_inventory(dungeon->player); break;
        }

        if (map->player->hp <= 0) {
            message_box("You have died...", "");
            return;
        }

        int key = con_getc();

        switch (key) {
        // debug related
            case 'M': // reveal map
                for (int y = 0; y < map->height; ++y) {
                    for (int x = 0; x < map->width; ++x) {
                        map->visibility[x + y * map->width] |= MAP_WAS_SEEN | MAP_IN_VIEW;
                    }
                }
                break;
            case 'H':
                map->player->hp = actor_get_stat(map->player, STAT_MAXHP);
                break;
            case 'L':
                message_add(map, "Message test.");
                break;
            case 'D':
                map_dump(map, "map.txt");
                break;
            case 'R': {
                int type = con_getc() - '0';
                int qty = con_getc() - '0';
                if (qty < 0 || qty > 9) {
                    message_format(map, "DEBUG: Bad item qty %d", qty);
                    break;
                }
                if (!actor_remove_items(dungeon->player, type, qty)) {
                    message_format(map, "DEBUG: Could not remove %d of type %d", qty, type);
                }
                break; }
            case 'O': {
                int type = con_getc() - '0';
                int count = actor_item_qty(dungeon->player, type);
                message_format(map, "DEBUG: holding %d of item type %d", count, type);
                break; }
            case 'I': {
                int type = con_getc() - '0';
                int qty = con_getc() - '0';
                if (qty < 0 || qty > 9) {
                    message_format(map, "DEBUG: Bad item qty %d", qty);
                    break;
                }
                struct item_def *item = item_new(type, qty);
                if (!item) {
                    message_format(map, "DEBUG: Failed to create item of type %d", type);
                    break;
                }
                if (!actor_add_item(dungeon->player, item)) {
                    message_format(map, "DEBUG: Could not add item!");
                    item_destroy(item);
                }
                break; }

        // change display
            case 'i':
                display_mode = 1;
                break;
            case 'c':
                display_mode = 0;
                break;

        // ability related
            case 'b':
                player_action(map, dungeon->player, ACTION_BASIC_ATTACK);
                break;

        //  movement related
            case ' ':
                player_action(map, dungeon->player, ACTION_WAIT);
                break;
            case CON_KEY_HOME:
                player_action(map, dungeon->player, ACTION_STEP_NORTHEAST);
                break;
            case CON_KEY_PAGEUP:
                player_action(map, dungeon->player, ACTION_STEP_NORTHWEST);
                break;
            case CON_KEY_END:
                player_action(map, dungeon->player, ACTION_STEP_SOUTHEAST);
                break;
            case CON_KEY_PAGEDOWN:
                player_action(map, dungeon->player, ACTION_STEP_SOUTHWEST);
                break;
            case CON_KEY_LEFT:
                player_action(map, dungeon->player, ACTION_STEP_EAST);
                break;
            case CON_KEY_RIGHT:
                player_action(map, dungeon->player, ACTION_STEP_WEST);
                break;
            case CON_KEY_UP:
                player_action(map, dungeon->player, ACTION_STEP_NORTH);
                break;
            case CON_KEY_DOWN:
                player_action(map, dungeon->player, ACTION_STEP_SOUTH);
                break;
            case 'W':
                if (!dungeon->complete && !yes_or_no("Abandon current run?", "You will receive no rewards if you leave now.", ANSWER_NO)) {
                    break;
                }
                map_set_actor(map, dungeon->player->x, dungeon->player->y, NULL);
                map_destroy(map);
                return;
            case 'Q':
                if (yes_or_no("Are you sure you want to quit?", "This will count as a withdraw from your current run.", ANSWER_NO) == ANSWER_NO) {
                    break;
                }
                wants_to_quit = 1;
                map_set_actor(map, dungeon->player->x, dungeon->player->y, NULL);
                map_destroy(map);
                return;
        }
    }

    map_set_actor(map, dungeon->player->x, dungeon->player->y, NULL);
    map_destroy(map);
}
