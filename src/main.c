#include "common.h"

static bool debug_mode = false;
static int num_threads = DEFAULT_THREADS;
static struct redis_config redis_conf = {
    .path = DEFAULT_REDIS_PATH,
    .is_unix = true,
    .password = DEFAULT_REDIS_PASSWD,
    .debug_mode = false
};
static volatile sig_atomic_t keep_running = 1;
static int server_fd = -1;

static void sigint_handler(int sig) {
    (void)sig;
    keep_running = 0;
    if (server_fd != -1) {
        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
        unlink(DEFAULT_SOCKET_PATH);
    }
}

int http_should_keep_running(void) {
    return keep_running;
}

int get_server_fd(void) {
    return server_fd;
}

void *thread_worker(void *arg) {
    set_redis_config(&redis_conf);
    http_serve();
    return NULL;
}

int main(int argc, char *argv[]) {
    if (!parse_args(argc, argv, &debug_mode, &num_threads, &redis_conf)) {
        return 1;
    }
    redis_conf.debug_mode = debug_mode;

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        print_debug("Socket creation failed: %s", strerror(errno));
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        print_debug("Setsockopt failed: %s", strerror(errno));
        close(server_fd);
        return 1;
    }

    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, DEFAULT_SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(DEFAULT_SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        print_debug("Bind failed on %s: %s", DEFAULT_SOCKET_PATH, strerror(errno));
        close(server_fd);
        return 1;
    }

    if (chmod(DEFAULT_SOCKET_PATH, 0666) == -1) {
        print_debug("Chmod failed on %s: %s", DEFAULT_SOCKET_PATH, strerror(errno));
        close(server_fd);
        unlink(DEFAULT_SOCKET_PATH);
        return 1;
    }

    if (listen(server_fd, 5) == -1) {
        print_debug("Listen failed: %s", strerror(errno));
        close(server_fd);
        unlink(DEFAULT_SOCKET_PATH);
        return 1;
    }

    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, sigint_handler);
    signal(SIGHUP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    if (!threads) {
        print_debug("Thread allocation failed");
        close(server_fd);
        unlink(DEFAULT_SOCKET_PATH);
        return 1;
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread_worker, NULL) != 0) {
            print_debug("Thread %d creation failed", i);
            free(threads);
            close(server_fd);
            unlink(DEFAULT_SOCKET_PATH);
            return 1;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    if (server_fd != -1) {
        close(server_fd);
        unlink(DEFAULT_SOCKET_PATH);
    }
    return 0;
}
