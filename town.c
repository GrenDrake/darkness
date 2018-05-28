#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "darkness.h"

struct town_def {
    struct actor_def *roster[MAX_ROSTER_SIZE];
    struct actor_def *hireable[MAX_HIREABLES];
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

        if (key >= '0' && key <= '9') {
            key -= '1';
            if (key < 0 || key >= MAX_ROSTER_SIZE) {
                continue;
            }
            return key;
        }

        key = tolower(key);
        if (key >= 'a' && key < 'a' + MAX_HIREABLES) {
            key -= 'a';
            if (key < 0 || key >= MAX_HIREABLES) {
                continue;
            }
            return key - 20;
        }
    }
    return -1;
}

#define NAME_COUNT  10
const char *male_names[NAME_COUNT] = {
    "Alan", "Bob", "Charlie", "David", "Evan", "Fred", "George", "Hal", "Ivan", "Joe"
};
const char *female_names[NAME_COUNT] = {
    "Abby", "Bev", "Claudia", "Dana", "Emily", "Francis", "Grace", "Heather", "Isabella", "Jane"
};

struct actor_def* generate_character() {
    struct actor_def *actor = actor_new(rng_max(2));
    if (actor == NULL) return actor;

    actor->sex = rng_max(2);
    int name_number = rng_max(NAME_COUNT);
    strcpy(actor->name, actor->sex == SEX_MALE ? male_names[name_number] : female_names[name_number]);
    return actor;
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

    for (int i = 0; i < MAX_HIREABLES; ++i) {
        if (town->hireable[i]) {
            actor_destroy(town->hireable[i]);
        }

        town->hireable[i] = generate_character();
    }
}


void roster_loop(struct town_def *town) {
    while (1) {
        int roster_count = 0;
        int hireable_count = 0;
        int empty_ranks = 0;
        int first_empty = -1;

        con_clear();
        for (int i = 0; i < MAX_ROSTER_SIZE; ++i) {
            struct actor_def *actor = town->roster[i];
            if (!actor) {
                if (first_empty < 0) {
                    first_empty = i;
                }
                ++empty_ranks;
                continue;
            }

            con_addstr(0, i, "%d) %s (%s)",
                i + 1,
                actor->name,
                actor->my_class->name);
            con_addstr(20, i, "HP: %d/%d",
                actor_get_stat(actor, STAT_HP),
                actor_get_stat(actor, STAT_MAXHP));
            ++roster_count;
        }

        for (int i = 0; i < MAX_HIREABLES; ++i) {
            struct actor_def *actor = town->hireable[i];
            if (!actor) continue;

            con_addstr(40, i, "%c) %s (%s)",
                i + 'A',
                actor->name,
                actor->my_class->name);
            con_addstr(60, i, "HP: %d/%d",
                actor_get_stat(actor, STAT_HP),
                actor_get_stat(actor, STAT_MAXHP));
            ++hireable_count;
        }

        if (hireable_count > 0 && empty_ranks > 0) {
            con_addstr(0, 17, "h) Hire character");
        }
        if (roster_count > 1) {
            con_addstr(0, 18, "d) Dismiss character");
        }
        con_addstr(0, 19, "s) Switch characters");
        con_addstr(0, 20, "z) Return");
        con_addstr(0, 21, "Q) Quit game");
        int key = con_getc();

        switch(key) {
            case 'h': {
                if (hireable_count <= 0 || empty_ranks <= 0) break;
                int who = pick_character("Character to hire");
                if (who >= -1) break;
                who += 20;
                if (town->hireable[who] == NULL) break;

                town->roster[first_empty] = town->hireable[who];
                town->hireable[who] = NULL;
                break; }
            case 'd': {
                if (roster_count <= 1) break;
                int first = pick_character("Pick character to dismiss");
                if (first <= -1) break;
                if (town->roster[first] == NULL) break;
                if (yes_or_no("Are you sure you to dismiss this character?", actor_get_name(town->roster[first]), ANSWER_NO) == ANSWER_NO) {
                    break;
                }
                actor_destroy(town->roster[first]);
                town->roster[first] = NULL;
                break; }

            case 's': {
                int first = pick_character("Pick first character");
                if (first <= -1) break;
                int second = pick_character("Pick second character");
                if (second <= -1) break;

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
    town.roster[0]->sex = SEX_MALE;
    strcpy(town.roster[0]->name, male_names[rng_max(NAME_COUNT)]);
    town.roster[1] = generate_character();
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
                if (who < 0 || town.roster[who] == NULL) {
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
                for (int i = 0; i < MAX_HIREABLES; ++i) {
                    free(town.hireable[i]);
                }
                return;
        }
    }
}