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

int roster_size(const struct town_def *town) {
    int size = 0;
    for (int i = 0; i < MAX_ROSTER_SIZE; ++i) {
        if (town->roster[i] != NULL) {
            ++size;
        }
    }
    return size;
}

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

int pick_character(const char *prompt) {
    con_clreol(0, 23);
    con_addstr(0, 23, prompt);
    con_addstr(strlen(prompt) + 1, 23, "(Z to cancel): ");

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

void process_new_week(struct town_def *town) {

    for (int i = 0; i < DUNGEON_COUNT; ++i) {
        if (town->dungeons[i].size != SIZE_NONE) {
            if (town->dungeons[i].complete || rng_max(100) >= 20) {
                town->dungeons[i].size = SIZE_NONE;
            }
        }

        if (town->dungeons[i].size == SIZE_NONE) {
            town->dungeons[i].complete = 0;
            town->dungeons[i].size = 1 + rng_max(SIZE_COUNT);
            town->dungeons[i].goal = rng_max(GOAL_COUNT);
        }
    }

    for (int i = 0; i < MAX_ROSTER_SIZE; ++i) {
        if (town->roster[i] == NULL) {
            continue;
        }

        int max_hp = actor_get_stat(town->roster[i], STAT_MAXHP);
        int hp_inc = max_hp / 5;
        town->roster[i]->hp += hp_inc;
        if (town->roster[i]->hp > max_hp) {
            town->roster[i]->hp = max_hp;
        }
    }
}


void roster_loop(struct town_def *town) {
    while (1) {
        int roster_count = roster_size(town);

        con_clear();
        for (int i = 0; i < MAX_ROSTER_SIZE; ++i) {
            struct actor_def *actor = town->roster[i];
            if (!actor) continue;

            con_addstr(0, i, "%d) %s (%s)",
                i + 1,
                actor->name,
                actor->my_class->name);
            con_addstr(20, i, "HP: %d/%d",
                actor_get_stat(actor, STAT_HP),
                actor_get_stat(actor, STAT_MAXHP));
        }

        con_addstr(0, 17, "h) Hire character");
        if (roster_count > 1) {
            con_addstr(0, 18, "d) Dismiss character");
        }
        con_addstr(0, 19, "s) Switch characters");
        con_addstr(0, 20, "z) Return");
        con_addstr(0, 21, "Q) Quit game");
        int key = con_getc();

        switch(key) {
            case 'd': {
                if (roster_count <= 1) break;
                int first = pick_character("Pick character to dismiss");
                if (first == -1) break;
                if (town->roster[first] == NULL) break;
                if (yes_or_no("Are you sure you to dismiss this character?", actor_get_name(town->roster[first]), ANSWER_NO) == ANSWER_NO) {
                    break;
                }
                actor_destroy(town->roster[first]);
                town->roster[first] = NULL;
                break; }

            case 's': {
                int first = pick_character("Pick first character");
                if (first == -1) break;
                int second = pick_character("Pick second character");
                if (second == -1) break;

                struct actor_def *tmp = town->roster[first];
                town->roster[first] = town->roster[second];
                town->roster[second] = tmp;
                break; }

            case 'Q':
                if (yes_or_no("Are you sure you want to quit?", "", ANSWER_NO) == ANSWER_NO) {
                    break;
                }
                wants_to_quit = 1;
                return;
            case 'z':
            case 'Z':
                return;
        }
    }
}


void town_loop() {
    struct town_def town = { { NULL } };
    int current_dungeon = 0;

    town.roster[0] = actor_new(0);
    town.roster[2] = actor_new(1);
    strcpy(town.roster[0]->name, "Fred");
    strcpy(town.roster[2]->name, "Jane");
    process_new_week(&town);

    while (!wants_to_quit) {
        con_clear();
        con_addstr(0, 0, "In town.");
        con_addstr(0, 1, "d - Delve");
        con_addstr(0, 2, "r - Roster");
        con_addstr(0, 3, "Q - Quit");
        con_addstr(0, 23, "Command: ");

        for (int i = 0; i < MAX_ROSTER_SIZE; ++i) {
            draw_character(i, town.roster[i]);
        }
        for (int i = 0; i < DUNGEON_COUNT; ++i) {
            con_addstr(25, i, "%s %d) %s %s %s",
                current_dungeon == i ? "**" : "  ", i + 1,
                size_names[town.dungeons[i].size - 1],
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
                int who = pick_character("Pick character to delve");
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
                process_new_week(&town);
                break; }

            case 'r':
                roster_loop(&town);
                break;

            case 'N':
                process_new_week(&town);
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