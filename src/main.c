#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "counters.h"

static void print_help(void) {
    printf("cmdline-counter\n");
    printf("Usage: \n");
    printf("  app [options] [counter]\n\n");
    printf("Options:\n");
    printf("  -f, --file=FILE        specify counters file\n");
    printf("  --set=VALUE, -s VALUE  set counter to value\n");
    printf("  --update=VALUE, -u VALUE increment counter by value\n");
    printf("  --delete, -d            delete counter\n");
    printf("  -h, --help, -?         show this help\n");
    printf("\n");
}

// Null-terminated array of pointers to counter structs
// counter **counters = NULL;

int main(int argc, char *argv[]) {
    const char *countersfile = NULL;
    const char *countername = NULL;
    const char *set_value = NULL;
    const char *update_value = NULL;
    int delete_flag = 0;

    // Check for help flags before processing other options
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-?") == 0) {
            print_help();
            return 0;
        }
    }

    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0) && i + 1 < argc) {
            countersfile = argv[i + 1];
            i++;
            continue;
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            countersfile = argv[i] + 7;
            continue;
        } else if ((strcmp(argv[i], "--set") == 0 || strcmp(argv[i], "-s") == 0) && i + 1 < argc) {
            set_value = argv[i + 1];
            i++;
            continue;
        } else if (strcmp(argv[i], "--update") == 0 && i + 1 < argc) {
            update_value = argv[i + 1];
            i++;
            continue;
        } else if (strcmp(argv[i], "-u") == 0 && i + 1 < argc) {
            update_value = argv[i + 1];
            i++;
            continue;
        } else if (strcmp(argv[i], "--delete") == 0 || strcmp(argv[i], "-d") == 0) {
            delete_flag = 1;
            continue;
        } else if (!countername) {
            countername = argv[i];
            continue;
        }
    }

    if (argc == 1) {
        print_help();
        return 0;
    }

    if (!countersfile) { // tag1
        countersfile = getenv("COUNTERS_FILE");
    }

    if (!countersfile) { // tag2
        const char *home = getenv("HOME");
        if (!home) home = ".";
        static char default_buf[1024];
        snprintf(default_buf, sizeof(default_buf), "%s/.counters", home);
        countersfile = default_buf;
    }

    read_counters(countersfile);

    

    // Counter existence handled in main logic
    // Handle counter operations
    if (countername) {
        // Ensure counter exists
        if (!counter_exists(countername)) {
            add_counter(countername, 0);
        }

        if (delete_flag) {
            delete_counter(countername);
        } else if (set_value) {
            long val = strtol(set_value, NULL, 10);
            size_t i;
            for (i = 0; counters[i]; i++) {
                if (strcmp(counters[i]->name, countername) == 0) {
                    counters[i]->count = (int)val;
                    break;
                }
            }
        } else if (update_value) {
            long delta = strtol(update_value, NULL, 10);
            size_t i;
            for (i = 0; counters[i]; i++) {
                if (strcmp(counters[i]->name, countername) == 0) {
                    counters[i]->count += (int)delta;
                    break;
                }
            }
        } else {
            size_t i;
            for (i = 0; counters[i]; i++) {
                if (strcmp(counters[i]->name, countername) == 0) {
                    printf("%d\n", counters[i]->count);
                    break;
                }
            }
        }
    }

    write_counters(countersfile);
    return 0;
}
