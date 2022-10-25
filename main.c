#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include "config.h"
#include "sayeth.h"

extern char wspace;
extern struct Driver **drivers;
extern size_t drivers_alloc;
extern size_t drivers_used;

/***
 * Repeat a character
 * @param ch character to repeat
 * @param limit number of times to repeat
 */
void repchar(char ch, size_t limit) {
    while (limit > 0) {
        putc(ch, stdout);
        limit--;
    }
}

/***
 * Find the longest line in a string
 * @param s the string
 * @return longest line
 */
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


void usage(char *prog) {
    printf("usage: %s [-d driver_name] [-w] [-] {input}\n"
           "-h      Show this help message\n"
           "-d      Driver name (fierrhea, jenkins, romero, doomguy, none)\n"
           "-w      Show indentation whitespace\n"
           "\n"
           "-       Read from standard input\n"
           "input   A quoted string\n", prog);
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
            case '?':
                fprintf(stderr, "unknown option: %c\n", optopt);
                usage(argv[0]);
                exit(1);
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
