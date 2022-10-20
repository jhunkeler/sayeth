#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "fierrhea.h"
#include "romero.h"
#include "jenkins.h"

#define INPUT_BUFSIZ 4096

size_t START_Y = 0;
const char *A_TAB = "    ";

void repchar(char ch, size_t limit) {
    while (limit > 0) {
        putc(ch, stdout);
        limit--;
    }
}

size_t get_longest_line(char *s) {
    size_t lengths[INPUT_BUFSIZ] = {0};
    size_t len;
    size_t line;

    len = 0;
    line = 0;
    for (size_t i = 0; i < strlen(s); i++) {
        if (s[i] == '\n') {
            lengths[line] = len;
            line++;
            len = 0;
            continue;
        }
        len++;
    }

    size_t longest;

    longest = lengths[0];
    for (size_t i = 0; i < sizeof(lengths) / sizeof(*lengths); i++) {
        if (lengths[i] > longest) {
            longest = lengths[i];
        }
    }
    return longest;
}

void box_draw_top(size_t y, size_t longest) {
    repchar(' ', y);
    repchar('/', 1);
    repchar('-', longest);
    repchar('\\', 1);
}

void box_draw_bottom(size_t y, size_t longest) {
    repchar(' ', y);
    repchar('\\', 1);
    repchar('-', longest);
    repchar('/', 1);
}

void box_draw_next_line(size_t y) {
    repchar(' ', y);
    repchar('|', 1);
}

void box_draw_end_line(size_t longest, size_t len) {
    repchar(' ', longest - len);
    repchar('|', 1);
}

int boxprintf(const char *fmt, ...) {
    int count;
    va_list list;
    va_start(list, fmt);
    size_t len = 0;
    char data[INPUT_BUFSIZ] = {0};
    char output[INPUT_BUFSIZ] = {0};
    vsnprintf(data, sizeof(data) - 1, fmt, list);

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

    box_draw_top(START_Y, longest);
    puts("");

    for (size_t i = 0; i < strlen(output); i++) {
        if (i == strlen(output) - 1) {
            box_draw_end_line(longest, len);
            continue;
        }

        if (len == 0) {
            box_draw_next_line(START_Y);
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
    box_draw_bottom(START_Y, longest);

    va_end(list);
    return count;
}

void driver_fierrhea(char *input) {
    START_Y = 22;
    printf("%s", FIERRHEA);
    printf("\\  /\n");
    printf("                      \\/\n");
    boxprintf(input);
    puts("");
}

void driver_jenkins(char *input) {
    START_Y = 18;
    boxprintf(input);
    puts("");
    printf("                   |  /\n");
    printf("                   | /\n");
    printf("                   |/\n");
    puts(JENKINS);
}

void driver_romero(char *input) {
    START_Y = 11;
    boxprintf(input);
    puts("");
    printf("            \\  |\n");
    printf("             \\ |\n");
    printf("              \\|\n");
    puts(ROMERO);
}

void usage(char *prog) {
    printf("usage: %s [-d driver_name] [-] {input}\n"
           "-h      Show this help message\n"
           "-d      Driver name (fierrhea, jenkins, romero)\n"
           "\n"
           "-       Read from standard input\n"
           "input   A quoted string\n", prog);
}

int main(int argc, char *argv[]) {
    char driver_name[255] = {0};
    char input[INPUT_BUFSIZ] = {0};
    char *iptr = input;

    // set default driver
    strcpy(driver_name, "fierrhea");

    int option;
    while ((option = getopt (argc, argv, "hd:")) != -1) {
        switch (option) {
            case 'h':
                usage(argv[0]);
                exit(0);
            case 'd':
                strcpy(driver_name, optarg);
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

    if (!strcmp(driver_name, "fierrhea")) {
        driver_fierrhea(input);
    } else if (!strcmp(driver_name, "jenkins")) {
        driver_jenkins(input);
    } else if (!strcmp(driver_name, "romero")) {
        driver_romero(input);
    } else {
        fprintf(stderr, "invalid driver: '%s'\n", driver_name);
        exit(1);
    }

    puts("");
    return 0;
}
