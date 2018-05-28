#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "darkness.h"

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
    struct actor_def *roster[MAX_ROSTER_SIZE] = { NULL };
    struct dungeon_def dungeons[DUNGEON_COUNT];
    int current_dungeon = 0;

    roster[0] = actor_new(0);
    roster[2] = actor_new(1);
    strcpy(roster[0]->name, "Fred");
    strcpy(roster[2]->name, "Jane");
    make_dungeons(dungeons);

    while (!wants_to_quit) {
        con_clear();
        con_addstr(0, 0, "In town.\nd - Delve\ns - Switch\nQ - Quit");
        con_addstr(0, 23, "Command: ");

        for (int i = 0; i < MAX_ROSTER_SIZE; ++i) {
            draw_character(i, roster[i]);
        }
        for (int i = 0; i < DUNGEON_COUNT; ++i) {
            con_addstr(25, i, "%s %d) %s %s %s",
                current_dungeon == i ? "**" : "  ", i + 1,
                size_names[dungeons[i].size],
                goal_names[dungeons[i].goal],
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
                if (roster[who] == NULL) {
                    break;
                }
                dungeons[current_dungeon].player = roster[who];
                dungeons[current_dungeon].complete = 0;
                delve_loop(&dungeons[current_dungeon]);
                if (roster[who]->hp <= 0) {
                    actor_destroy(roster[who]);
                    roster[who] = NULL;
                }
                break; }

            case 's': {
                int first = pick_character();
                if (first == -1) break;
                int second = pick_character();
                if (second == -1) break;

                struct actor_def *tmp = roster[first];
                roster[first] = roster[second];
                roster[second] = tmp;
                break; }

            case 'N':
                make_dungeons(dungeons);
                break;

            case 'Q':
                if (yes_or_no("Are you sure you want to quit?", "", ANSWER_NO) == ANSWER_NO) {
                    break;
                }
                wants_to_quit = 1;
                for (int i = 0; i < MAX_ROSTER_SIZE; ++i) {
                    free(roster[i]);
                }
                return;
        }
    }
}