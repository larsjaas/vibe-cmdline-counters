#include "counters.h"

#include <stdio.h>
#include <stdlib.h>

counter **counters = NULL;

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
    counters = malloc((capacity + 1) * sizeof(counter *));
    if (!counters) {
        fclose(fp);
        return; // allocation failed
    }

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

// Check if a counter with the given name exists
int counter_exists(const char *name) {
    if (!counters) return 0;
    for (counter **p = counters; *p; p++) {
        if (strcmp((*p)->name, name) == 0) return 1;
    }
    return 0;
}

// Add a new counter with the specified initial count
int add_counter(const char *name, int count) {
    if (!name) return 0;
    size_t sz = 0;
    while (counters && counters[sz]) sz++;

    counter *cnt = malloc(sizeof(counter));
    if (!cnt) return 0;
    cnt->name = malloc(strlen(name) + 1);
    if (!cnt->name) {
        free(cnt);
        return 0;
    }
    strcpy(cnt->name, name);
    cnt->count = count;

    counter **new_arr = realloc(counters, (sz + 2) * sizeof(counter *));
    if (!new_arr) {
        free(cnt->name);
        free(cnt);
        return 0;
    }
    counters = new_arr;
    counters[sz] = cnt;
    counters[sz + 1] = NULL;
    return 1;
}

// Delete the counter with the specified name
int delete_counter(const char *name) {
    if (!name || !counters) return 0;
    size_t found = (size_t)-1;
    for (size_t i = 0; counters[i]; i++) {
        if (strcmp(counters[i]->name, name) == 0) {
            found = i;
            break;
        }
    }
    if (found == (size_t)-1) return 0;

    free(counters[found]->name);
    free(counters[found]);

    size_t j = found;
    while (counters[j + 1]) {
        counters[j] = counters[j + 1];
        j++;
    }
    counters[j] = NULL;
    return 1;
}

// Write counters to a CSV-like file
void write_counters(const char *filename) {
    if (!filename) return;
    FILE *fp = fopen(filename, "w");
    if (!fp) return;
    for (counter **ptr = counters; ptr && *ptr; ptr++) {
        fprintf(fp, "%s,%d\n", (*ptr)->name, (*ptr)->count);
    }
    fclose(fp);
}
