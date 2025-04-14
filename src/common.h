#ifndef COMMON_H
#define COMMON_H

#define REQUEST_MAX_SIZE 1024
#define SIGNUP_OK_TTL 3600
#define SIGNUP_FAILED_TTL 3610
#define SIGNLOCK_TTL 30
#define USER_TTL 2592000
#define DEFAULT_SOCKET_PATH "/wwwFS.out/unix.signup.sock"
#define DEFAULT_REDIS_PATH "/wwwFS.in/u98/unix.redis.sock"
#define DEFAULT_THREADS 4
#define MAX_THREADS 8
#define MIN_THREADS 1
#define REDIS_TIMEOUT_SEC 2
#define USERNAME_MIN_LEN 8
#define USERNAME_MAX_LEN 20
// notice : sha256sum, result is 32 bytes, hex is 64 chars.
#define PASSWD_LEN 64
#define SALT_LEN 32
#define HASH_LEN 96
#define DEFAULT_REDIS_PASSWD "default_captcha_passwd"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/stat.h>
#include <jansson.h>
#include <ctype.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <hiredis/hiredis.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>
#include <stdbool.h>

struct signup_request {
    char *username;
    char *passwd;
};

struct redis_config {
    char *path;
    bool is_unix;
    char *password;
    bool debug_mode;
};

void print_debug(const char *fmt, ...);
void http_print_debug(const char *fmt, ...);
void print_help(void);
bool parse_args(int argc, char *argv[], bool *debug, int *threads, struct redis_config *redis);
bool redis_check_ip(const char *ip, struct redis_config *conf);
bool redis_check_username(const char *username, struct redis_config *conf);
bool redis_store_user(const char *username, const char *hash, const char *salt, const char *ip, struct redis_config *conf);
bool redis_increment_failed(const char *ip, struct redis_config *conf);
bool compute_hash(const char *username, const char *passwd, char *hash, char *salt);
void set_redis_config(struct redis_config *conf);
void http_serve(void);
int http_should_keep_running(void);
int get_server_fd(void);

#endif
