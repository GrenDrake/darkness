#include <ctype.h>
#include <ncurses.h>
#include <string.h>

#include "darkness.h"

int yes_or_no(const char *prompt, const char *line2, int default_answer) {
    unsigned offset;
    int maxX = 0, maxY = 0;
    size_t prompt_length = strlen(prompt);
    size_t line2_length = strlen(line2);
    size_t longest_line = prompt_length > line2_length
                            ? prompt_length
                            : line2_length;
    getmaxyx(stdscr, maxY, maxX);

    unsigned windowWidth = longest_line + 6;
    unsigned windowHeight = 6;
    unsigned top  = (maxY - windowHeight) / 2;
    unsigned left = (maxX - windowWidth)  / 2;

    bkgdset(A_NORMAL | COLOR_PAIR(CP_DIALOG));
    for (unsigned y = 0; y < windowHeight; ++y) {
        for (unsigned x = 0; x < windowWidth; ++x) {
            mvaddch(y + top, x + left, ' ');
        }
        mvaddch(y + top, left, ACS_VLINE);
        mvaddch(y + top, left + windowWidth - 1, ACS_VLINE);
    }
    mvaddch(top, left, ACS_ULCORNER);
    mvaddch(top, left + windowWidth - 1, ACS_URCORNER);
    mvaddch(top + windowHeight - 1, left, ACS_LLCORNER);
    mvaddch(top + windowHeight - 1, left+windowWidth - 1, ACS_LRCORNER);
    offset = (longest_line - prompt_length) / 2;
    mvprintw(top + 1, left + 3 + offset, "%s", prompt);
    offset = (longest_line - line2_length) / 2;
    mvprintw(top + 2, left + 3 + offset, "%s", line2);
    switch(default_answer) {
        case ANSWER_YES:        mvprintw(top+4, (maxX - 10) / 2, "YES or no?"); break;
        case ANSWER_NO:         mvprintw(top+4, (maxX - 10) / 2, "yes or NO?"); break;
        case ANSWER_NODEFAULT:  mvprintw(top+4, (maxX - 10) / 2, "yes or no?"); break;
    }

    refresh();
    while (true) {
        int key = toupper(getch());
        if (key == 'Y') return ANSWER_YES;
        if (key == 'N') return ANSWER_NO;
        if (default_answer != ANSWER_NODEFAULT && (key == KEY_ENTER || key == ' ' || key == '\n' || key == '\r')) {
            return default_answer;
        }
    }
}

void message_box(const char *line1, const char *line2) {
    unsigned offset;
    int maxX = 0, maxY = 0;
    size_t prompt_length = strlen(line1);
    size_t line2_length = strlen(line2);
    size_t longest_line = prompt_length > line2_length
                            ? prompt_length
                            : line2_length;
    getmaxyx(stdscr, maxY, maxX);

    unsigned windowWidth = longest_line + 6;
    unsigned windowHeight = 6;
    unsigned top  = (maxY - windowHeight) / 2;
    unsigned left = (maxX - windowWidth)  / 2;

    bkgdset(A_NORMAL | COLOR_PAIR(CP_DIALOG));
    for (unsigned y = 0; y < windowHeight; ++y) {
        for (unsigned x = 0; x < windowWidth; ++x) {
            mvaddch(y + top, x + left, ' ');
        }
        mvaddch(y + top, left, ACS_VLINE);
        mvaddch(y + top, left + windowWidth - 1, ACS_VLINE);
    }
    mvaddch(top, left, ACS_ULCORNER);
    mvaddch(top, left + windowWidth - 1, ACS_URCORNER);
    mvaddch(top + windowHeight - 1, left, ACS_LLCORNER);
    mvaddch(top + windowHeight - 1, left+windowWidth - 1, ACS_LRCORNER);
    offset = (longest_line - prompt_length) / 2;
    mvprintw(top + 1, left + 3 + offset, "%s", line1);
    offset = (longest_line - line2_length) / 2;
    mvprintw(top + 2, left + 3 + offset, "%s", line2);
    mvprintw(top+4, (maxX - 4) / 2, "OKAY");

    refresh();
    toupper(getch());
}
