#ifndef COMMON_H
#define COMMON_H

/* Constants */
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
#define PASSWD_LEN 64
#define SALT_LEN 32
#define HASH_LEN 96
#define DEFAULT_REDIS_PASSWD "default_captcha_passwd"

/* Standard library includes */
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
#include <argon2.h>
#include <openssl/rand.h>

/* Data structures */
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

/* Function declarations */

/* Debug and utility functions */
void print_debug(const char *fmt, ...);
void http_print_debug(const char *fmt, ...);
void print_help(void);
bool parse_args(int argc, char *argv[], bool *debug, int *threads, struct redis_config *redis);

/* Redis functions */
bool redis_check_ip(const char *ip, struct redis_config *conf);
bool redis_check_username(const char *username, struct redis_config *conf);
bool redis_store_user(const char *username, const char *hash, const char *salt, const char *ip, struct redis_config *conf);
bool redis_increment_failed(const char *ip, struct redis_config *conf);
bool redis_connect_thread(struct redis_config *conf);

/* Hash computation */
bool compute_hash(const char *username, const char *passwd, char *hash, char *salt);

/* HTTP and server functions */
void set_redis_config(struct redis_config *conf);
void http_serve(void);
int http_should_keep_running(void);
int get_server_fd(void);
const char *extract_client_ip(const char *http_request);
bool parse_json(const char *body, struct signup_request *req);
bool validate_username(const char *username);
bool validate_passwd(const char *passwd);
void send_response(int client_fd, int status, const char *status_text, const char *return_context);
char *get_client_ip(int client_fd, const char *buffer);

/* Thread functions */
void *thread_worker(void *arg);

bool redis_find_signup_sess_and_reset_its_TTL300(const char *signup_sess, struct redis_config *conf) ;
bool redis_save_signup_sess_with_TTL300(const char *signup_sess, struct redis_config *conf) ;
void gen_a_new_md5sum_hex_32byte(char *output) ;
void send_response_with_new_signup_sess(int client_fd, int status, const char *signup_sess) ;

#endif /* COMMON_H */
