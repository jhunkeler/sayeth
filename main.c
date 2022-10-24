#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include "config.h"

#define INPUT_BUFSIZ 4096
#define DATA_BUFSIZ 8192
#define DRIVERS_ALLOC_DEFAULT 128

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

struct Driver {
    char *name;
    size_t box_indent;
    char *box_elements;
    size_t caret_pos;
    size_t caret_indent;
    size_t caret_attached;
    char *caret;
    size_t data_indent;
    char *data;
};
struct Driver **drivers = NULL;
size_t drivers_alloc = DRIVERS_ALLOC_DEFAULT;
size_t drivers_used = 0;


void repchar(char ch, size_t limit) {
    while (limit > 0) {
        putc(ch, stdout);
        limit--;
    }
}

size_t get_longest_line(char *s) {
    size_t *lengths;
    size_t len;
    size_t linecount;
    size_t longest;
    char *ch = s;

    linecount = 0;
    while (*ch != '\0') {
        if (*ch == '\n')
            linecount++;
        ch++;
    }

    lengths = calloc(linecount + 1, sizeof(*lengths));
    if (!lengths) {
        return ULONG_MAX;
    }

    len = 0;
    for (size_t i = 0, line = 0; i < strlen(s); i++) {
        if (s[i] == '\n') {
            lengths[line] = len;
            line++;
            len = 0;
            continue;
        }
        len++;
    }

    longest = lengths[0];
    for (size_t i = 0; i < linecount; i++) {
        if (lengths[i] > longest) {
            longest = lengths[i];
        }
    }
    free(lengths);
    return longest;
}

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

int boxprintf(const char *fmt, ...) {
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

void usage(char *prog) {
    printf("usage: %s [-d driver_name] [-w] [-] {input}\n"
           "-h      Show this help message\n"
           "-d      Driver name (fierrhea, jenkins, romero, doomguy, none)\n"
           "-w      Show indentation whitespace\n"
           "\n"
           "-       Read from standard input\n"
           "input   A quoted string\n", prog);
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
        char buf[INPUT_BUFSIZ] = {0};
        ssize_t lastpos = ftell(fp);
        if (fgets(buf, sizeof(buf) - 1, fp) == NULL) {
            break;
        }
        if (!strlen(buf) || buf[0] == '#') {
            i--;
            continue;
        }
        buf[strlen(buf) - 1] = '\0';
        switch (i) {
            case 0:
                driver->name = strdup(buf);
                if (!driver->name) {
                    fclose(fp);
                    return NULL;
                }
                break;
            case 1:
                driver->box_indent = strtol(buf, NULL, 10);
                break;
            case 2:
                driver->box_elements = strdup(buf);
                if (!driver->box_elements) {
                    fclose(fp);
                    return NULL;
                }
                break;
            case 3:
                driver->caret_pos = strtol(buf, NULL, 10);
                break;
            case 4:
                driver->caret_indent = strtol(buf, NULL, 10);
                break;
            case 5:
                driver->caret_attached = strtol(buf, NULL, 10);
                break;
            case 6:
                driver->caret = strdup(buf);
                if (!driver->caret) {
                    fclose(fp);
                    return NULL;
                }
                break;
            case 7:
                driver->data_indent = strtol(buf, 0, 10);
                break;
            case 8:
                driver->data = calloc(DATA_BUFSIZ, sizeof(*driver->data));
                if (!driver->data) {
                    fclose(fp);
                    return NULL;
                }
                // rewind to beginning of the data section
                fseek(fp, lastpos, SEEK_SET);
                fread(driver->data, 1, DATA_BUFSIZ - 1, fp);
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

void drivers_free() {
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

void driver_run(struct Driver *driver, char *input) {
    char *elem = driver->box_elements;
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
        boxprintf("%s", input);
        caret_draw(driver->caret, START_Y_CARET, driver->caret_attached);
    }

    if (driver->data) {
        data_draw(driver->data, START_Y_DATA);
    }

    if (driver->caret_pos) {
        caret_draw(driver->caret, START_Y_CARET, driver->caret_attached);
        boxprintf("%s", input);
    }
}

struct Driver *driver_lookup(char *name) {
    for (size_t i = 0; drivers[i] != NULL; i++) {
        if (!strcmp(drivers[i]->name, name)) {
            return drivers[i];
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    char driver_name[255] = {0};
    char input[INPUT_BUFSIZ] = {0};
    struct Driver *driver = NULL;
    char *driver_dir = getenv("SAYETH_DRIVERS");
    if (!driver_dir) {
#ifdef DRIVER_DIR
        driver_dir = strdup(DRIVER_DIR);
#else
        driver_dir = strdup("./drivers");
#endif
    }

    // set default driver
    strcpy(driver_name, "fierrhea");

    int option;
    while ((option = getopt (argc, argv, "hd:w")) != -1) {
        switch (option) {
            case 'h':
                usage(argv[0]);
                exit(0);
            case 'd':
                strcpy(driver_name, optarg);
                break;
            case 'w':
                wspace = '.';
                break;
            case ':':
                fprintf(stderr, "option requires value\n");
                usage(argv[0]);
                exit(1);
                break;
            case '?':
                fprintf(stderr, "unknown option: %c\n", optopt);
                usage(argv[0]);
                exit(1);
                break;
            default:
                usage(argv[0]);
                exit(1);
        }
    }

    DIR *dir;
    struct dirent *dp;

    dir = opendir(driver_dir);
    if (!dir) {
        perror(driver_dir);
        exit(1);
    }

    while ((dp = readdir(dir)) != NULL) {
        struct Driver *drv;
        char path[PATH_MAX] = {0};
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, driver_dir)) {
            continue;
        }
        if (!strstr(dp->d_name, ".art")) {
            continue;
        }
        snprintf(path, sizeof(path) - 1, "%s/%s", driver_dir, dp->d_name);
        drv = driver_load(path);
        if (!drv) {
            fprintf(stderr, "unable to load driver: %s (%s)\n", path, strerror(errno));
            exit(1);
        }
        if (driver_register(drv) < 0) {
            fprintf(stderr, "unable to register driver: %s (%s)\n", path, strerror(errno));
            exit(1);
        }
        free(drv);
    }
    closedir(dir);

    if (!drivers_used) {
        fprintf(stderr, "No drivers present in '%s'?\n", driver_dir);
        exit(1);
    }

    if (optind < argc) {
        strcpy(input, argv[optind]);
    } else {
        fprintf(stderr, "Missing input string or standard input (`-`)\n");
        usage(argv[0]);
        exit(1);
    }

    if (input[0] == '-') {
       fread(input, 1, sizeof(input) - 1, stdin);
    }

    if (input[strlen(input) - 1] != '\n') {
        input[strlen(input)] = '\n';
    }

    driver = driver_lookup(driver_name);
    if (!driver) {
        fprintf(stderr, "Driver not found\n");
        exit(1);
    }

    driver_run(driver, input);
    puts("");
    drivers_free();
    free(driver_dir);
    return 0;
}
