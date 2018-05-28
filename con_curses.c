#include <ncurses.h>

#include "console.h"

#define MAX_STRING_LENGTH   128

int con_init() {
    initscr();
    noecho();
    keypad(stdscr, true);
    cbreak();
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
    init_pair(15, COLOR_WHITE, COLOR_BLACK);
    return 1;
}

void con_end() {
    endwin();
}


int con_maxx() {
    return getmaxx(stdscr);
}

int con_maxy() {
    return getmaxy(stdscr);
}


int con_clear() {
    clear();
    return 1;
}

int con_addstr(int x, int y, const char *text, ...) {
    char msg[MAX_STRING_LENGTH] = "";

    va_list args;
    va_start(args, text);
    vsnprintf(msg, MAX_STRING_LENGTH - 1, text, args);
    va_end(args);

    mvprintw(y, x, msg);
    return 1;
}

int con_addchar(int x, int y, int character) {
    mvaddch(y, x, character);
    return 1;
}

int con_clreol(int x, int y) {
    move(y, x);
    clrtoeol();
    return 1;
}

int con_getc() {
    refresh();
    int key = getch();
    switch(key) {
        case KEY_LEFT:  return CON_KEY_LEFT;
        case KEY_RIGHT: return CON_KEY_RIGHT;
        case KEY_UP:    return CON_KEY_UP;
        case KEY_DOWN:  return CON_KEY_DOWN;
        case KEY_ENTER: return '\n';
        case KEY_HOME:  return CON_KEY_HOME;
        case KEY_END:   return CON_KEY_END;
        case KEY_NPAGE: return CON_KEY_PAGEDOWN;
        case KEY_PPAGE: return CON_KEY_PAGEUP;
        default: return key;
    }
}

int con_setattr(int attribute) {
    if (attribute & CON_BOLD) {
        attron(A_BOLD);
    }
    return 1;
}

int con_setcolour(int colour) {
    attron(COLOR_PAIR(colour));
    return 1;
}

int con_setbgcolour(int colour) {
    bkgdset(A_NORMAL | COLOR_PAIR(colour));
    return 1;
}

int con_resetattr() {
    attrset(0);
    return 1;
}

int con_special(int which) {
    switch(which) {
        case CON_CHAR_HLINE:    return ACS_HLINE;
        case CON_CHAR_VLINE:    return ACS_VLINE;
        case CON_CHAR_LLCORNER: return ACS_LLCORNER;
        case CON_CHAR_LRCORNER: return ACS_LRCORNER;
        case CON_CHAR_ULCORNER: return ACS_ULCORNER;
        case CON_CHAR_URCORNER: return ACS_URCORNER;
        case CON_CHAR_CHECKER:  return ACS_CKBOARD;
        default: return '?';
    }
}