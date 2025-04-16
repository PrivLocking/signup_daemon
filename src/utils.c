#include "common.h"

void print_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

static void print_defaults(bool print_current, bool debug_mode, struct redis_config *redis, int *threads) {
    const char *defaults[] = {
        "DEFAULT_SOCKET_PATH",
        "DEFAULT_REDIS_PATH",
        "DEFAULT_THREADS",
        "DEFAULT_REDIS_PASSWD",
        NULL
    };

    if (!print_current) {
        printf("Default Parameters:\n");
    } else {
        printf("Current Parameter Values:\n");
    }

    for (int i = 0; defaults[i]; i++) {
        if (strcmp(defaults[i], "DEFAULT_SOCKET_PATH") == 0) {
            printf("%s = %s\n", defaults[i], print_current ? DEFAULT_SOCKET_PATH : DEFAULT_SOCKET_PATH);
        } else if (strcmp(defaults[i], "DEFAULT_REDIS_PATH") == 0) {
            printf("%s = %s\n", defaults[i], print_current ? redis->path : DEFAULT_REDIS_PATH);
        } else if (strcmp(defaults[i], "DEFAULT_THREADS") == 0) {
            printf("%s = %d\n", defaults[i], print_current ? *threads : DEFAULT_THREADS);
        } else if (strcmp(defaults[i], "DEFAULT_REDIS_PASSWD") == 0) {
            printf("%s = %s\n", defaults[i], print_current ? redis->password : DEFAULT_REDIS_PASSWD);
        }
    }
}

void print_help(void) {
    printf("Usage: signup_daemon [-h/--help] [-d/--debug] [-t threads] [-r redis_path] [-u] [-p password]\n");
    printf("Options:\n");
    printf("  -h, --help      Show this help message\n");
    printf("  -d, --debug     Enable debug mode\n");
    printf("  -t threads      Number of threads (1-8, default %d)\n", DEFAULT_THREADS);
    printf("  -r path         Redis UNIX socket path (default %s)\n", DEFAULT_REDIS_PATH);
    printf("  -u              Use UNIX socket for Redis (default)\n");
    printf("  -p password     Redis password\n");
    print_defaults(false, false, NULL, NULL);
    exit(0);
}

bool parse_args(int argc, char *argv[], bool *debug, int *threads, struct redis_config *redis) {
    int opt;
    *debug = false;
    *threads = DEFAULT_THREADS;
    redis->password = strdup(DEFAULT_REDIS_PASSWD);

    char *env_passwd = getenv("REDIS_PASSWD");
    if (env_passwd) {
        free(redis->password);
        redis->password = strdup(env_passwd);
    }

    while ((opt = getopt(argc, argv, "hdt:r:up:")) != -1) {
        switch (opt) {
        case 'h':
            print_help();
            break;
        case 'd':
            *debug = true;
            break;
        case 't':
            *threads = atoi(optarg);
            if (*threads < MIN_THREADS || *threads > MAX_THREADS) {
                fprintf(stderr, "Threads must be between %d and %d\n", MIN_THREADS, MAX_THREADS);
                return false;
            }
            break;
        case 'r':
            redis->path = strdup(optarg);
            break;
        case 'u':
            redis->is_unix = true;
            break;
        case 'p':
            free(redis->password);
            redis->password = strdup(optarg);
            break;
        default:
            print_help();
            return false;
        }
    }

    if (!redis->path) redis->path = strdup(DEFAULT_REDIS_PATH);
    if (*debug) {
        print_defaults(true, *debug, redis, threads);
    }
    return true;
}
