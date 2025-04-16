#include "common.h"

static volatile sig_atomic_t keep_running = 1;
static int server_fd = -1;

void sigint_handler(int sig) {
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

void set_server_fd(int fd) {
    server_fd = fd;
}

int setup_server_socket(void) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        print_debug("Socket creation failed: %s", strerror(errno));
        return -1;
    }

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        print_debug("Setsockopt failed: %s", strerror(errno));
        close(fd);
        return -1;
    }

    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, DEFAULT_SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(DEFAULT_SOCKET_PATH);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        print_debug("Bind failed on %s: %s", DEFAULT_SOCKET_PATH, strerror(errno));
        close(fd);
        return -1;
    }

    if (chmod(DEFAULT_SOCKET_PATH, 0666) == -1) {
        print_debug("Chmod failed on %s: %s", DEFAULT_SOCKET_PATH, strerror(errno));
        close(fd);
        unlink(DEFAULT_SOCKET_PATH);
        return -1;
    }

    if (listen(fd, 5) == -1) {
        print_debug("Listen failed: %s", strerror(errno));
        close(fd);
        unlink(DEFAULT_SOCKET_PATH);
        return -1;
    }

    set_server_fd(fd);
    return fd;
}

void *thread_worker(void *arg) {
    set_redis_config((struct redis_config *)arg);
    http_serve();
    return NULL;
}

void setup_signal_handlers(void) {
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, sigint_handler);
    signal(SIGHUP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
}
