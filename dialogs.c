#include <ctype.h>
#include <string.h>

#include "console.h"
#include "darkness.h"

int yes_or_no(const char *prompt, const char *line2, int default_answer) {
    unsigned offset;
    int maxX = con_maxx();
    int maxY = con_maxy();
    size_t prompt_length = strlen(prompt);
    size_t line2_length = strlen(line2);
    size_t longest_line = prompt_length > line2_length
                            ? prompt_length
                            : line2_length;

    unsigned windowWidth = longest_line + 6;
    unsigned windowHeight = 6;
    unsigned top  = (maxY - windowHeight) / 2;
    unsigned left = (maxX - windowWidth)  / 2;

    con_setbgcolour(CP_DIALOG);
    for (unsigned y = 0; y < windowHeight; ++y) {
        for (unsigned x = 0; x < windowWidth; ++x) {
            con_addchar(x + left, y + top, ' ');
        }
        con_addchar(left, y + top, con_special(CON_CHAR_VLINE));
        con_addchar(left + windowWidth - 1, y + top, con_special(CON_CHAR_VLINE));
    }
    con_addchar(left, top, con_special(CON_CHAR_ULCORNER));
    con_addchar(left + windowWidth - 1, top, con_special(CON_CHAR_URCORNER));
    con_addchar(left, top + windowHeight - 1, con_special(CON_CHAR_LLCORNER));
    con_addchar(left + windowWidth - 1, top + windowHeight - 1, con_special(CON_CHAR_LRCORNER));
    offset = (longest_line - prompt_length) / 2;
    con_addstr(left + 3 + offset, top + 1, "%s", prompt);
    offset = (longest_line - line2_length) / 2;
    con_addstr(left + 3 + offset, top + 2, "%s", line2);
    switch(default_answer) {
        case ANSWER_YES:        con_addstr((maxX - 10) / 2, top + 4, "YES or no?"); break;
        case ANSWER_NO:         con_addstr((maxX - 10) / 2, top + 4, "yes or NO?"); break;
        case ANSWER_NODEFAULT:  con_addstr((maxX - 10) / 2, top + 4, "yes or no?"); break;
    }

    while (1) {
        int key = toupper(con_getc());
        if (key == 'Y') return ANSWER_YES;
        if (key == 'N') return ANSWER_NO;
        if (default_answer != ANSWER_NODEFAULT && (key == ' ' || key == '\n' || key == '\r')) {
            return default_answer;
        }
    }
}

void message_box(const char *line1, const char *line2) {
    unsigned offset;
    int maxX = con_maxx();
    int maxY = con_maxy();
    size_t prompt_length = strlen(line1);
    size_t line2_length = strlen(line2);
    size_t longest_line = prompt_length > line2_length
                            ? prompt_length
                            : line2_length;

    unsigned windowWidth = longest_line + 6;
    unsigned windowHeight = 6;
    unsigned top  = (maxY - windowHeight) / 2;
    unsigned left = (maxX - windowWidth)  / 2;

    con_setbgcolour(CP_DIALOG);
    for (unsigned y = 0; y < windowHeight; ++y) {
        for (unsigned x = 0; x < windowWidth; ++x) {
            con_addchar(x + left, y + top, ' ');
        }
        con_addchar(left, y + top, con_special(CON_CHAR_VLINE));
        con_addchar(left + windowWidth - 1, y + top, con_special(CON_CHAR_VLINE));
    }
    con_addchar(left, top, con_special(CON_CHAR_ULCORNER));
    con_addchar(left + windowWidth - 1, top, con_special(CON_CHAR_URCORNER));
    con_addchar(left, top + windowHeight - 1, con_special(CON_CHAR_LLCORNER));
    con_addchar(left+windowWidth - 1, top + windowHeight - 1, con_special(CON_CHAR_LRCORNER));
    offset = (longest_line - prompt_length) / 2;
    con_addstr(left + 3 + offset, top + 1, "%s", line1);
    offset = (longest_line - line2_length) / 2;
    con_addstr(left + 3 + offset, top + 2, "%s", line2);
    con_addstr((maxX - 4) / 2, top + 4, "OKAY");

    con_getc();
}
