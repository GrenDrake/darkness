#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "darkness.h"

struct town_def {
    struct actor_def *roster[MAX_ROSTER_SIZE];
    struct dungeon_def dungeons[DUNGEON_COUNT];
};

int wants_to_quit = 0;

void draw_character(int position, const struct actor_def *actor) {
    con_addstr(60, position * 3 + 2, "-= =-  -= =-  -= =-");
    if (!actor) return;
    con_addstr(60, position * 3, "%d) %s (%s)",
        position + 1,
        actor->name,
        actor->my_class->name);
    con_addstr(65, position * 3 + 1, "HP: %d/%d",
        actor_get_stat(actor, STAT_HP),
        actor_get_stat(actor, STAT_MAXHP));
}

int pick_character() {
    con_clreol(0, 23);
    con_addstr(0, 23, "Select character (Z to cancel): ");

    while (1) {
        int key = con_getc();

        if (key == 'Z') {
            return -1;
        }

        key -= '1';
        if (key < 0 || key >= MAX_ROSTER_SIZE) {
            continue;
        }

        return key;
    }
    return -1;
}

void make_dungeons(struct dungeon_def *dungeons) {
    for (int i = 0; i < DUNGEON_COUNT; ++i) {
        dungeons[i].size = rng_max(SIZE_COUNT);
        dungeons[i].goal = rng_max(GOAL_COUNT);
    }
}

void town_loop() {
    struct town_def town = { { NULL } };
    int current_dungeon = 0;

    town.roster[0] = actor_new(0);
    town.roster[2] = actor_new(1);
    strcpy(town.roster[0]->name, "Fred");
    strcpy(town.roster[2]->name, "Jane");
    make_dungeons(town.dungeons);

    while (!wants_to_quit) {
        con_clear();
        con_addstr(0, 0, "In town.\nd - Delve\ns - Switch\nQ - Quit");
        con_addstr(0, 23, "Command: ");

        for (int i = 0; i < MAX_ROSTER_SIZE; ++i) {
            draw_character(i, town.roster[i]);
        }
        for (int i = 0; i < DUNGEON_COUNT; ++i) {
            con_addstr(25, i, "%s %d) %s %s %s",
                current_dungeon == i ? "**" : "  ", i + 1,
                size_names[town.dungeons[i].size],
                goal_names[town.dungeons[i].goal],
                current_dungeon == i ? "**" : "  ");
        }

        int key = con_getc();

        switch (key) {
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
                current_dungeon = key - '1';
                break;

            case 'd': {
                int who = pick_character();
                if (town.roster[who] == NULL) {
                    break;
                }
                town.dungeons[current_dungeon].player = town.roster[who];
                town.dungeons[current_dungeon].complete = 0;
                delve_loop(&town.dungeons[current_dungeon]);
                if (town.roster[who]->hp <= 0) {
                    actor_destroy(town.roster[who]);
                    town.roster[who] = NULL;
                }
                break; }

            case 's': {
                int first = pick_character();
                if (first == -1) break;
                int second = pick_character();
                if (second == -1) break;

                struct actor_def *tmp = town.roster[first];
                town.roster[first] = town.roster[second];
                town.roster[second] = tmp;
                break; }

            case 'N':
                make_dungeons(town.dungeons);
                break;

            case 'Q':
                if (yes_or_no("Are you sure you want to quit?", "", ANSWER_NO) == ANSWER_NO) {
                    break;
                }
                wants_to_quit = 1;
                for (int i = 0; i < MAX_ROSTER_SIZE; ++i) {
                    free(town.roster[i]);
                }
                return;
        }
    }
}