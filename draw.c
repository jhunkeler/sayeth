#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <wchar.h>
#include "sayeth.h"

size_t START_Y_BOX = 0;
size_t START_Y_CARET = 0;
size_t START_Y_DATA = 0;
const wchar_t *A_TAB = L"    ";
wchar_t wspace = L' ';
wchar_t box_top_left = L'\0';
wchar_t box_top = L'\0';
wchar_t box_top_right = L'\0';
wchar_t box_side = L'\0';
wchar_t box_bottom_left = L'\0';
wchar_t box_bottom = L'\0';
wchar_t box_bottom_right = L'\0';
extern int do_fill;

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

int box_printf(const wchar_t *fmt, ...) {
    int count;
    va_list list;
    va_start(list, fmt);
    size_t len = 0;
    wchar_t data[INPUT_BUFSIZ] = {0};
    wchar_t output[INPUT_BUFSIZ] = {0};
    count = vswprintf(data, sizeof(data) - 1, fmt, list);

    // convert tabs to spaces
    for (size_t i = 0, n = 0; i < wcslen(data) && n < sizeof(output); i++, n++) {
        if (data[i] == L'\t') {
            wcscat(&output[n], A_TAB);
            n = n + wcslen(A_TAB) - 1;
            continue;
        }
        output[n] = data[i];
    }

    size_t longest = get_longest_line(output);

    box_draw_top(START_Y_BOX, longest);
    wprintf(L"\n");

    for (size_t i = 0; i < wcslen(output); i++) {
        if (i == wcslen(output) - 1) {
            box_draw_end_line(longest, len);
            continue;
        }

        if (len == 0) {
            box_draw_next_line(START_Y_BOX);
        }

        if (output[i] == '\n') {
            box_draw_end_line(longest, len);
            putwc('\n', stdout);
            len = 0;
            continue;
        }

        putwc(output[i], stdout);

        len++;
    }
    wprintf(L"\n");
    box_draw_bottom(START_Y_BOX, longest);

    va_end(list);
    return count;
}

void caret_draw(wchar_t *data, size_t indent, size_t attached) {
    if (!wcslen(data))
        return;

    if (!attached)
        repchar(wspace, START_Y_BOX);

    for (size_t i = 0; i < wcslen(data); i++) {
        if (data[i] == L'\\' && isalpha(data[i + 1])) {
            i++;
            switch (data[i]) {
                case 'n':
                    putwc(L'\n', stdout);
                    if (i == wcslen(data) - 1)
                        continue;
                    repchar(wspace, indent);
                    continue;
                default:
                    continue;
            }
        }
        putwc(data[i], stdout);
    }
}

size_t count_lines(wchar_t *s) {
    size_t count = 0;
    wchar_t *ptr = s;

    while (*ptr != L'\0') {
        if (*ptr == L'\n')
            count++;
        ptr++;
    }
    return count;
}

void data_draw(wchar_t *data, size_t indent) {
    size_t len = 0;
    wchar_t *inbuf = wcsdup(data);
    size_t longest = get_longest_line(inbuf);
    size_t lines = count_lines(inbuf);
    wchar_t buf[DATA_BUFSIZ] = {0};

    wchar_t *token = NULL;
    wchar_t *ptr;
    token = wcstok(inbuf, L"\n", &ptr);
    for (size_t i = 0; token != NULL; i++) {
        memset(buf, '\0', sizeof(buf));
        for (size_t y = 0; y < indent; y++) {
            buf[y] = wspace;
        }
        wcscat(buf, token);
        len = wcslen(buf);
        if (i < lines) {
            for (size_t y = len; y < longest + (indent * 2); y++) {
                buf[y] = wspace;
            }
            wcscat(buf, L"\n");
        }
        wprintf(L"%S", buf);
        token = wcstok(NULL, L"\n", &ptr);
    }
}
