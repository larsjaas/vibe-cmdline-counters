#ifndef COUNTERS_H
#define COUNTERS_H

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct counter {
    char *name;
    int count;
} counter;

// Global counter array
extern counter **counters;

// Function prototypes
void read_counters(const char *filename);
void write_counters(const char *filename);
int counter_exists(const char *name);
int add_counter(const char *name, int count);
int delete_counter(const char *name);

#ifdef __cplusplus
}
#endif

#endif // COUNTERS_H