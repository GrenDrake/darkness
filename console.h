#ifndef CONSOLE_H
#define CONSOLE_H

#define CON_BOLD            1

#define CON_KEY_LEFT        0
#define CON_KEY_RIGHT       1
#define CON_KEY_UP          2
#define CON_KEY_DOWN        3
#define CON_KEY_HOME        4
#define CON_KEY_END         5
#define CON_KEY_PAGEUP      6
#define CON_KEY_PAGEDOWN    7

#define CON_CHAR_VLINE      0
#define CON_CHAR_HLINE      1
#define CON_CHAR_ULCORNER   2
#define CON_CHAR_URCORNER   3
#define CON_CHAR_LLCORNER   4
#define CON_CHAR_LRCORNER   5
#define CON_CHAR_CHECKER    6


int con_init();
void con_end();

int con_clear();
int con_addstr(int x, int y, const char *text, ...);
int con_addchar(int x, int y, int character);
int con_clreol(int x, int y);
int con_getc();

int con_maxx();
int con_maxy();

int con_setattr(int attribute);
int con_setcolour(int colour);
int con_setbgcolour(int colour);
int con_resetattr();

int con_special(int which);

#endif
