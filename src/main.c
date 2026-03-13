#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Counter structure
typedef struct counter {
    char *name;
    int count;
} counter;

// Null-terminated array of pointers to counter structs
counter **counters = NULL;

// Write counters to a CSV-like file
void write_counters(const char *filename) {
    if (!filename) return;
    FILE *fp = fopen(filename, "w");
    if (!fp) return;
    counter **ptr = counters;
    while (ptr && *ptr) {
        fprintf(fp, "%s,%d\n", (*ptr)->name, (*ptr)->count);
        ptr++;
    }
    fclose(fp);
}

// Read counters from a CSV-like file
void read_counters(const char *filename) {
    if (!filename) return;
    FILE *fp = fopen(filename, "r");
    if (!fp) return;

    // Free any previously allocated counters
    if (counters) {
        for (counter **p = counters; *p; p++) {
            free((*p)->name);
            free(*p);
        }
        free(counters);
        counters = NULL;
    }

    size_t count = 0;
    size_t capacity = 8; // initial capacity
    counters = malloc((capacity + 1) * sizeof(counter *)); // +1 for NULL terminator
    if (!counters) return; // allocation failed

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        // Remove trailing newline if present
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') continue; // skip empty lines

        char *comma = strchr(line, ',');
        if (!comma) continue; // malformed line

        *comma = '\0';
        char *name_part = line;
        char *count_part = comma + 1;

        counter *cnt = malloc(sizeof(counter));
        if (!cnt) break;
        cnt->name = malloc(strlen(name_part) + 1);
        if (cnt->name) {
            memcpy(cnt->name, name_part, strlen(name_part) + 1);
        } else {
            free(cnt);
            break;
        }
        if (!cnt->name) {
            free(cnt);
            break;
        }
        cnt->count = atoi(count_part);

        if (count >= capacity) {
            capacity *= 2;
            counter **new_arr = realloc(counters, (capacity + 1) * sizeof(counter *));
            if (!new_arr) {
                free(cnt->name);
                free(cnt);
                break;
            }
            counters = new_arr;
        }
        counters[count++] = cnt;
        counters[count] = NULL; // keep NULL terminator
    }
    fclose(fp);
}

int main(int argc, char *argv[]) {
    const char *countersfile = NULL;
    const char *countername = NULL;
    const char *set_value = NULL;
    const char *update_value = NULL;
    int delete_flag = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            countersfile = argv[i + 1];
            i++;
            continue;
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            countersfile = argv[i] + 7;
            continue;
        } else if (strcmp(argv[i], "--file") == 0 && i + 1 < argc) {
            countersfile = argv[i + 1];
            i++;
            continue;
        } else if (strcmp(argv[i], "--set") == 0 && i + 1 < argc) {
            set_value = argv[i + 1];
            i++;
            continue;
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
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
        } else if (strcmp(argv[i], "--delete") == 0) {
            delete_flag = 1;
            continue;
        } else if (!countername) {
            countername = argv[i];
            continue;
        }
    }

    if (!countersfile) {
        countersfile = getenv("COUNTERS_FILE");
    }

    if (!countersfile) {
        const char *home = getenv("HOME");
        if (!home) home = ".";
        static char default_buf[1024];
        snprintf(default_buf, sizeof(default_buf), "%s/.counters", home);
        countersfile = default_buf;
    }

    read_counters(countersfile);

    printf("Hello world!\n");

    write_counters(countersfile);
    return 0;
}
