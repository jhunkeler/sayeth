#ifndef SAYETH_SAYETH_H
#define SAYETH_SAYETH_H
#include <stdlib.h>

// Maximum length of input text
#define INPUT_BUFSIZ 4096

// Maximum length of art data
#define DATA_BUFSIZ 8192

// Seed global driver array with n records
#define DRIVERS_ALLOC_DEFAULT 128

struct Driver {
    char *name;            // Driver name (passed to '-d' argument)
    size_t box_indent;     // Shift box n columns to the right
    char *box_elements;    // Border characters
                           // top left, top, top right, sides, bottom left, bottom, bottom right
                           // e.g. "/-\|\-/'
                           /*
                            * /--------\
                            * |        |
                            * \--------/
                            */
    size_t caret_pos;      // Above or below the box
    size_t caret_indent;   // Shift caret n columns to the right
    size_t caret_attached; // Inject a new line *after* art data, or not?
    char *caret;           // Caret characters
                           // e.g. "\n \  |\n  \ |\n   \|\n"
                           /*
                            * \  |
                            *  \ |
                            *   \|
                            */
    size_t data_indent;    // Shift art data n columns to the right
    char *data;            // Art data (up to size: DATA_BUFSIZ - 1)
};

void repchar(char ch, size_t limit);
size_t get_longest_line(char *s);

struct Driver *driver_load(char *filename);
struct Driver *driver_lookup(char *name);
int driver_register(struct Driver *driver);
void driver_run(struct Driver *driver, char *input);
void drivers_free(void);

void box_draw_top(size_t y, size_t longest);
void box_draw_bottom(size_t y, size_t longest);
void box_draw_next_line(size_t y);
void box_draw_end_line(size_t longest, size_t len);
int box_printf(const char *fmt, ...);

void caret_draw(char *data, size_t indent, size_t attached);
void data_draw(char *data, size_t indent);
#endif //SAYETH_SAYETH_H
