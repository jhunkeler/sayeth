#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "sayeth.h"

size_t START_Y_BOX = 0;
size_t START_Y_CARET = 0;
size_t START_Y_DATA = 0;
const char *A_TAB = "    ";
char wspace = ' ';
char box_top_left = '\0';
char box_top = '\0';
char box_top_right = '\0';
char box_side = '\0';
char box_bottom_left = '\0';
char box_bottom = '\0';
char box_bottom_right = '\0';

void box_draw_top(size_t y, size_t longest) {
    repchar(wspace, y);
    repchar(box_top_left, 1);
    repchar(box_top, longest);
    repchar(box_top_right, 1);
}

void box_draw_bottom(size_t y, size_t longest) {
    repchar(wspace, y);
    repchar(box_bottom_left, 1);
    repchar(box_bottom, longest);
    repchar(box_bottom_right, 1);
}

void box_draw_next_line(size_t y) {
    repchar(wspace, y);
    repchar(box_side, 1);
}

void box_draw_end_line(size_t longest, size_t len) {
    repchar(wspace, longest - len);
    repchar(box_side, 1);
}

int box_printf(const char *fmt, ...) {
    int count;
    va_list list;
    va_start(list, fmt);
    size_t len = 0;
    char data[INPUT_BUFSIZ] = {0};
    char output[INPUT_BUFSIZ] = {0};
    count = vsnprintf(data, sizeof(data) - 1, fmt, list);

    // convert tabs to spaces
    for (size_t i = 0, n = 0; i < strlen(data) && n < sizeof(output); i++, n++) {
        if (data[i] == '\t') {
            strcat(&output[n], A_TAB);
            n = n + strlen(A_TAB) - 1;
            continue;
        }
        output[n] = data[i];
    }

    size_t longest = get_longest_line(output);

    box_draw_top(START_Y_BOX, longest);
    puts("");

    for (size_t i = 0; i < strlen(output); i++) {
        if (i == strlen(output) - 1) {
            box_draw_end_line(longest, len);
            continue;
        }

        if (len == 0) {
            box_draw_next_line(START_Y_BOX);
        }

        if (output[i] == '\n') {
            box_draw_end_line(longest, len);
            putc('\n', stdout);
            len = 0;
            continue;
        }

        putc(output[i], stdout);

        len++;
    }
    puts("");
    box_draw_bottom(START_Y_BOX, longest);

    va_end(list);
    return count;
}

void caret_draw(char *data, size_t indent, size_t attached) {
    if (!strlen(data))
        return;

    if (!attached)
        repchar(wspace, START_Y_BOX);

    for (size_t i = 0; i < strlen(data); i++) {
        if (data[i] == '\\' && isalpha(data[i + 1])) {
            i++;
            switch (data[i]) {
                case 'n':
                    putc('\n', stdout);
                    if (i == strlen(data) - 1)
                        continue;
                    repchar(wspace, indent);
                    continue;
                default:
                    continue;
            }
        }
        putc(data[i], stdout);
    }
}


void data_draw(char *data, size_t indent) {
    size_t len = 0;
    for (size_t i = 0; i < strlen(data); i++) {
        if (len == 0) {
            repchar(wspace, indent);
        }

        if (data[i] == '\n') {
            putc('\n', stdout);
            len = 0;
            continue;
        }

        putc(data[i], stdout);
        len++;
    }
}
