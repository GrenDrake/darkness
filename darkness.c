#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ncurses.h>

#include "darkness.h"

void gameloop();
void delve_loop(struct dungeon_def *dungeon);

int wants_to_quit = 0;

void draw_character(int position, const struct actor_def *actor) {
    mvprintw(position * 3 + 2, 60, "-= =-  -= =-  -= =-");
    if (!actor) return;
    mvprintw(position * 3, 60, "%d) %s (%s)",
        position + 1,
        actor->name,
        actor->my_class->name);
    mvprintw(position * 3 + 1, 65, "HP: %d/%d",
        actor_get_stat(actor, STAT_HP),
        actor_get_stat(actor, STAT_MAXHP));
}

int pick_character() {
    move(23, 0);
    clrtoeol();
    printw("Select character (Z to cancel): ");

    while (1) {
        int key = getch();
        addch(key);

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

void gameloop() {
    struct actor_def *roster[MAX_ROSTER_SIZE] = { NULL };
    struct dungeon_def dungeons[DUNGEON_COUNT];
    int current_dungeon = 0;

    roster[0] = actor_new(0);
    roster[2] = actor_new(1);
    strcpy(roster[0]->name, "Fred");
    strcpy(roster[2]->name, "Jane");
    make_dungeons(dungeons);

    while (!wants_to_quit) {
        clear();
        printw("In town.\nd - Delve\ns - Switch\nQ - Quit");
        mvprintw(23, 0, "Command: ");

        for (int i = 0; i < MAX_ROSTER_SIZE; ++i) {
            draw_character(i, roster[i]);
        }
        for (int i = 0; i < DUNGEON_COUNT; ++i) {
            mvprintw(i, 25, "%s %d) %s %s %s",
                current_dungeon == i ? "**" : "  ", i + 1,
                size_names[dungeons[i].size],
                goal_names[dungeons[i].goal],
                current_dungeon == i ? "**" : "  ");
        }

        int key = getch();

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
                delve_loop(&dungeons[current_dungeon]);
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
                wants_to_quit = 1;
                return;
        }
    }
}


int main() {
    initscr();
    noecho();
    keypad(stdscr, true);
    cbreak();
    rng_init_time();
    gameloop();
    endwin();
    return 0;
}
