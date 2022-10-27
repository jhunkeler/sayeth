#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "sayeth.h"

extern int do_fill;
extern size_t START_Y_BOX;
extern size_t START_Y_CARET;
extern size_t START_Y_DATA;
extern const char *A_TAB;
extern char wspace;
extern char box_top_left;
extern char box_top;
extern char box_top_right;
extern char box_side;
extern char box_bottom_left;
extern char box_bottom;
extern char box_bottom_right;

struct Driver **drivers = NULL;
size_t drivers_alloc = DRIVERS_ALLOC_DEFAULT;
size_t drivers_used = 0;

struct Driver *driver_load(char *filename) {
    FILE *fp = NULL;
    struct Driver *driver = NULL;

    driver = calloc(1, sizeof(*driver));
    if (!driver) {
        return NULL;
    }

    fp = fopen(filename, "r");
    if (!fp) {
        return NULL;
    }

    for (size_t i = 0; i < 9; i++) {
        wchar_t buf[INPUT_BUFSIZ] = {0};
        ssize_t lastpos = ftell(fp);
        if (fgetws(buf, sizeof(buf) - 1, fp) == NULL) {
            break;
        }
        if (!wcslen(buf) || buf[0] == '#') {
            i--;
            continue;
        }
        buf[wcslen(buf) - 1] = '\0';
        switch (i) {
            case 0:
                driver->name = wcsdup(buf);
                if (!driver->name) {
                    fclose(fp);
                    return NULL;
                }
                break;
            case 1:
                driver->box_indent = wcstol(buf, NULL, 10);
                break;
            case 2:
                driver->box_elements = wcsdup(buf);
                if (!driver->box_elements) {
                    fclose(fp);
                    return NULL;
                }
                break;
            case 3:
                driver->caret_pos = wcstol(buf, NULL, 10);
                break;
            case 4:
                driver->caret_indent = wcstol(buf, NULL, 10);
                break;
            case 5:
                driver->caret_attached = wcstol(buf, NULL, 10);
                break;
            case 6:
                driver->caret = wcsdup(buf);
                if (!driver->caret) {
                    fclose(fp);
                    return NULL;
                }
                break;
            case 7:
                driver->data_indent = wcstol(buf, 0, 10);
                break;
            case 8:
                driver->data = calloc(DATA_BUFSIZ, sizeof(*driver->data));
                if (!driver->data) {
                    fclose(fp);
                    return NULL;
                }
                fseek(fp, lastpos, SEEK_SET);
                wint_t c = 0;
                size_t n = 0;
                while ((c = fgetwc(fp)) != WEOF) {
                    driver->data[n++] = (wchar_t) c;
                }
                break;
            default:
                break;
        }
    }
    fclose(fp);
    return driver;
}

int driver_register(struct Driver *driver) {
    if (!drivers) {
        drivers = calloc(drivers_alloc, sizeof(**drivers));
    }

    if (drivers_used > drivers_alloc) {
        struct Driver **tmp;
        drivers_alloc += DRIVERS_ALLOC_DEFAULT;
        tmp = realloc(drivers, (sizeof(**drivers) * drivers_alloc));
        if (!tmp) {
            return -1;
        }
        drivers = tmp;

        for (size_t i = drivers_used; i < drivers_alloc; i++) {
            drivers[i] = calloc(1, sizeof(*drivers[i]));
            if (!drivers[i])
                return -2;
        }
    }

    drivers[drivers_used] = calloc(1, sizeof(*drivers[0]));
    if (!drivers[drivers_used])
        return -2;
    memcpy(drivers[drivers_used], driver, sizeof(*drivers[0]));
    drivers_used++;
    return 0;
}

void drivers_free(void) {
    for (size_t i = 0; i < drivers_alloc; i++) {
        if (drivers[i]) {
            if (drivers[i]->name)
                free(drivers[i]->name);
            if (drivers[i]->data)
                free(drivers[i]->data);
            if (drivers[i]->caret)
                free(drivers[i]->caret);
            if (drivers[i]->box_elements)
                free(drivers[i]->box_elements);
            free(drivers[i]);
        }
    }
    free(drivers);
}
void driver_run(struct Driver *driver, wchar_t *input) {
    wchar_t *elem = driver->box_elements;
    box_top_left = elem[0];
    box_top = elem[1];
    box_top_right = elem[2];
    box_side = elem[3];
    box_bottom_left = elem[4];
    box_bottom = elem[5];
    box_bottom_right = elem[6];
    START_Y_BOX = driver->box_indent;
    START_Y_CARET = driver->caret_indent;
    START_Y_DATA = driver->data_indent;

    if (!driver->caret_pos) {
        box_printf(L"%S", input);
        caret_draw(driver->caret, START_Y_CARET, driver->caret_attached);
    }

    if (driver->data) {
        data_draw(driver->data, START_Y_DATA);
    }

    if (driver->caret_pos) {
        caret_draw(driver->caret, START_Y_CARET, driver->caret_attached);
        box_printf(L"%S", input);
    }
}

struct Driver *driver_lookup(wchar_t *name) {
    for (size_t i = 0; drivers[i] != NULL; i++) {
        if (!wcscmp(drivers[i]->name, name)) {
            return drivers[i];
        }
    }
    return NULL;
}

