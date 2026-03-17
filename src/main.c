/*
 * cmdline-counter main program.
 * Implements counter operations and optional timestamp override for logs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "counters.h"

/* Override timestamp for logs; 0 means use current time. */
static long long timestamp_override = 0;

/* Get current time in milliseconds since epoch. */
static long long get_epoch_milliseconds(void)
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) {
        return 0;
    }
    return (long long)tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
}

static void print_help(void)
{
    printf("cmdline-counter\n");
    printf("Usage:\n");
    printf("  app [options] [counter]\n\n");
    printf("Options:\n");
    printf("  -f, --file=FILE        specify counters file (default: $COUNTERS_FILE or $HOME/.counters)\n");
    printf("  --set=VALUE, -s VALUE  set counter to VALUE\n");
    printf("  --update=VALUE, -u VALUE increment counter by VALUE\n");
    printf("  --delete, -d           delete counter\n");
    printf("  -l, --log[=FILE]       specify log file (default: $COUNTERS_LOG or $HOME/.counters.log)\n");
    printf("  -t, --timestamp=TS     override timestamp for logs (milliseconds)\n");
    printf("  -h, --help, -?         show this help\n\n");
    printf("Environment variables:\n");
    printf("  COUNTERS_FILE          default counter file path\n");
    printf("  COUNTERS_LOG           default log file path\n");
    printf("  HOME                   used for default paths when env vars are unset\n");
}

/* Helper to compute default counters file path. */
static const char *default_counters_file(void)
{
    const char *env = getenv("COUNTERS_FILE");
    if (env) {
        return env;
    }
    const char *home = getenv("HOME");
    if (!home) home = ""; /* fallback */
    static char buf[1024];
    snprintf(buf, sizeof(buf), "%s/.counters", home);
    return buf;
}

static const char *default_log_file(void)
{
    const char *env = getenv("COUNTERS_LOG");
    if (env) {
        return env;
    }
    const char *home = getenv("HOME");
    if (!home) home = "";
    static char buf[1024];
    snprintf(buf, sizeof(buf), "%s/.counters.log", home);
    return buf;
}

int main(int argc, char *argv[])
{
    const char *countersfile = NULL;
    const char *counters_log = NULL;
    const char *countername = NULL;
    const char *set_value = NULL;
    const char *update_value = NULL;
    int delete_flag = 0;
    FILE *logfp = NULL;

    /* Check for help flag early. */
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0 || strcmp(arg, "-?") == 0) {
            print_help();
            return 0;
        }
    }

    /* Default file paths. */
    countersfile = default_counters_file();
    counters_log = default_log_file();

    /* Parse command line options. */
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        /* File options */
        if ((strcmp(arg, "-f") == 0 || strcmp(arg, "--file") == 0) && i + 1 < argc) {
            countersfile = argv[++i];
            continue;
        }
        if (strncmp(arg, "--file=", 7) == 0) {
            countersfile = arg + 7;
            continue;
        }
        /* Log file option (unused but kept) */
        if ((strcmp(arg, "-l") == 0 || strcmp(arg, "--log") == 0) && i + 1 < argc) {
            counters_log = argv[++i];
            continue;
        }
        if (strncmp(arg, "--log=", 6) == 0) {
            counters_log = arg + 6;
            continue;
        }
        /* Set value */
        if ((strcmp(arg, "-s") == 0 || strcmp(arg, "--set") == 0) && i + 1 < argc) {
            set_value = argv[++i];
            continue;
        }
        if (strncmp(arg, "--set=", 6) == 0) {
            set_value = arg + 6;
            continue;
        }
        /* Update value */
        if ((strcmp(arg, "-u") == 0 || strcmp(arg, "--update") == 0) && i + 1 < argc) {
            update_value = argv[++i];
            continue;
        }
        if (strncmp(arg, "--update=", 9) == 0) {
            update_value = arg + 9;
            continue;
        }
        /* Delete flag */
        if (strcmp(arg, "-d") == 0 || strcmp(arg, "--delete") == 0) {
            delete_flag = 1;
            continue;
        }
        /* Timestamp override */
        if ((strcmp(arg, "-t") == 0 || strcmp(arg, "--timestamp") == 0) && i + 1 < argc) {
            timestamp_override = strtoll(argv[++i], NULL, 10);
            continue;
        }
        if (strncmp(arg, "--timestamp=", 12) == 0) {
            timestamp_override = strtoll(arg + 12, NULL, 10);
            continue;
        }
        /* Non-option argument assumed to be counter name if not already given. */
        if (!countername && arg[0] != '-') {
            countername = arg;
            continue;
        }
    }

    /* If no arguments were provided, show help. */
    if (argc == 1) {
        print_help();
        return 0;
    }

    /* Open log file if requested. */
    if (counters_log) {
        logfp = fopen(counters_log, "a");
        if (!logfp) {
            fprintf(stderr, "Warning: cannot open log file '%s' for appending.\n", counters_log);
        }
    }

    /* Load counters from file. */
    read_counters(countersfile);

    /* Apply operations on the specified counter if any. */
    if (countername) {
        /* Ensure counter exists (create if not). */
        if (!counter_exists(countername)) {
            int init_val = 0;
            add_counter(countername, init_val);
            if (logfp) {
                long long epoch = timestamp_override ? timestamp_override : get_epoch_milliseconds();
                fprintf(logfp, "new,%s,%lld,%lld\n", countername, init_val, epoch);
            }
        }

        if (delete_flag) {
            delete_counter(countername);
            if (logfp) {
                long long epoch = timestamp_override ? timestamp_override : get_epoch_milliseconds();
                fprintf(logfp, "delete,%s,%lld\n", countername, epoch);
            }
        } else if (set_value) {
            long long val = strtoll(set_value, NULL, 10);
            int idx = 0;
            while (counters[idx]) {
                if (strcmp(counters[idx]->name, countername) == 0) {
                    counters[idx]->count = (int)val;
                    break;
                }
                ++idx;
            }
            if (logfp) {
                long long epoch = timestamp_override ? timestamp_override : get_epoch_milliseconds();
                fprintf(logfp, "set,%s,%lld,%lld\n", countername, val, epoch);
            }
        } else if (update_value) {
            long long delta = strtoll(update_value, NULL, 10);
            int idx = 0;
            while (counters[idx]) {
                if (strcmp(counters[idx]->name, countername) == 0) {
                    counters[idx]->count += (int)delta;
                    break;
                }
                ++idx;
            }
            if (logfp) {
                long long epoch = timestamp_override ? timestamp_override : get_epoch_milliseconds();
                fprintf(logfp, "update,%s,%lld,%lld\n", countername, delta, epoch);
            }
        } else {
            /* Print current value. */
            int idx = 0;
            while (counters[idx]) {
                if (strcmp(counters[idx]->name, countername) == 0) {
                    printf("%d\n", counters[idx]->count);
                    break;
                }
                ++idx;
            }
        }
    }

    /* Persist counters to file. */
    write_counters(countersfile);

    if (logfp) {
        fflush(logfp);
        fclose(logfp);
    }

    return 0;
}
