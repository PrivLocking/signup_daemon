#include "common.h"

extern int setup_server_socket(void);
extern void setup_signal_handlers(void);
extern void *thread_worker(void *arg);

int main(int argc, char *argv[]) {
    bool debug_mode = false;
    int num_threads = DEFAULT_THREADS;
    struct redis_config redis_conf = {
        .path = DEFAULT_REDIS_PATH,
        .is_unix = true,
        .password = DEFAULT_REDIS_PASSWD,
        .debug_mode = false
    };

    if (!parse_args(argc, argv, &debug_mode, &num_threads, &redis_conf)) {
        return 1;
    }
    redis_conf.debug_mode = debug_mode;

    print_debug("Executable file's md5 is [%s]\n", get_executable_md5_hex() );

    if (setup_server_socket() == -1) {
        return 1;
    }

    setup_signal_handlers();

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    if (!threads) {
        print_debug("Thread allocation failed");
        close(get_server_fd());
        unlink(DEFAULT_SOCKET_PATH);
        return 1;
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread_worker, &redis_conf) != 0) {
            print_debug("Thread %d creation failed", i);
            free(threads);
            close(get_server_fd());
            unlink(DEFAULT_SOCKET_PATH);
            return 1;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    if (get_server_fd() != -1) {
        close(get_server_fd());
        unlink(DEFAULT_SOCKET_PATH);
    }
    return 0;
}
