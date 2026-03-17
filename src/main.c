#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/time.h>

#include "counters.h"

/* Get current time in milliseconds since epoch */
static long long get_epoch_milliseconds(void)
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1) {
        return 0;
    }
    return (long long)tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;
}

/* Print help message. */
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
    printf("  -h, --help, -?         show this help\n\n");
    printf("Environment variables:\n");
    printf("  COUNTERS_FILE          default counter file path\n");
    printf("  COUNTERS_LOG           default log file path\n");
    printf("  HOME                   used for default paths when env vars are unset\n");
}

int main(int argc, char *argv[])
{
    /* --- Argument / option handling ------------------------------ */
    const char *countersfile = NULL;
    const char *counters_log = NULL;    /* log file name, unused in current code */
    const char *countername = NULL;
    const char *set_value = NULL;
    const char *update_value = NULL;
    int delete_flag = 0;
    FILE *logfp = NULL;

    /* If help flag present anywhere, show help and exit. */
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-?") == 0) {
            print_help();
            return 0;
        }
    }

    /* Determine defaults for counters file */
    countersfile = getenv("COUNTERS_FILE");
    if (!countersfile) {
        const char *home = getenv("HOME");
        if (!home) home = "";  /* fallback: empty string */
        static char default_buf[1024];
        snprintf(default_buf, sizeof(default_buf), "%s/.counters", home);
        countersfile = default_buf;
    }

    /* Determine defaults for log file (unused in this version) */
    counters_log = getenv("COUNTERS_LOG");
    if (!counters_log) {
        const char *home = getenv("HOME");
        if (!home) home = "";
        static char default_log_buf[1024];
        snprintf(default_log_buf, sizeof(default_log_buf), "%s/.counters.log", home);
        counters_log = default_log_buf;
    }
    /* log file logic not implemented yet. */

    /* Parse command line options */
    for (int i = 1; i < argc; ++i) {
        if ((strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0) && i + 1 < argc) {
            countersfile = argv[i + 1];
            ++i; continue;
        }
        if (strncmp(argv[i], "--file=", 7) == 0) {
            countersfile = argv[i] + 7; continue;
        }
        if ((strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--set") == 0) && i + 1 < argc) {
            set_value = argv[i + 1]; ++i; continue;
        }
        if (strncmp(argv[i], "--set=", 6) == 0) {
            set_value = argv[i] + 6; continue;
        }
        if ((strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--update") == 0) && i + 1 < argc) {
            update_value = argv[i + 1]; ++i; continue;
        }
        if (strncmp(argv[i], "--update=", 9) == 0) {
            update_value = argv[i] + 9; continue;
        }
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delete") == 0) {
            delete_flag = 1; continue;
        }
        if ((strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--log") == 0) && i + 1 < argc) {
            counters_log = argv[i + 1]; ++i; continue;
        }
        if (strncmp(argv[i], "--log=", 6) == 0) {
            counters_log = argv[i] + 6; continue;
        }
        /* If not an option, treat as counter name if not already set */
        if (!countername && (argv[i][0] != '-')) {
            countername = argv[i]; continue;
        }
    }

    /* If no arguments were provided, show help and exit. */
    if (argc == 1) {
        print_help();
        return 0;
    }

    /* Ensure counters file is valid before reading */
    if (!countersfile) {
        printf("Error: counters file not specified.\n");
        return 1;
    }

    /* Open log file for appending */
    if (counters_log) {
        logfp = fopen(counters_log, "a");
        if (!logfp) {
            fprintf(stderr, "Warning: cannot open log file '%s' for appending.\n", counters_log);
        }
    }

    /* Read counters from file */
    read_counters(countersfile);

    /* If a counter name was provided, perform operations */
    if (countername) {
        /* Ensure counter exists */
        if (!counter_exists(countername)) {
            long init_val = 0;
            add_counter(countername, init_val);
            if (logfp) {
                long long epoch = get_epoch_milliseconds();
                fprintf(logfp, "new,%s,%lld,%lld\n", countername, init_val, epoch);
            }
        }
        if (delete_flag) {
            delete_counter(countername);
            if (logfp) {
                long long epoch = get_epoch_milliseconds();
                fprintf(logfp, "delete,%s,%lld\n", countername, epoch);
            }
        } else if (set_value) {
            long val = strtol(set_value, NULL, 10);
            int idx = 0;
            while (counters[idx]) {
                if (strcmp(counters[idx]->name, countername) == 0) {
                    counters[idx]->count = (int)val;
                    break;
                }
                ++idx;
            }
            if (logfp) {
                long long epoch = get_epoch_milliseconds();
                fprintf(logfp, "set,%s,%lld,%lld\n", countername, val, epoch);
            }
        } else if (update_value) {
            long delta = strtol(update_value, NULL, 10);
            int idx = 0;
            while (counters[idx]) {
                if (strcmp(counters[idx]->name, countername) == 0) {
                    counters[idx]->count += (int)delta;
                    break;
                }
                ++idx;
            }
            if (logfp) {
                long long epoch = get_epoch_milliseconds();
                fprintf(logfp, "update,%s,%lld,%lld\n", countername, delta, epoch);
            }
        } else {
            /* Print the current value */
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

    /* Write counters back to file */
    write_counters(countersfile);

    /* Flush and close log file */
    if (logfp) {
        fflush(logfp);
        fclose(logfp);
    }

    return 0;
}
