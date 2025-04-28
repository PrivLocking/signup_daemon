#ifndef COMMON_H
#define COMMON_H

#define _GNU_SOURCE         /* See feature_test_macros(7) */
/* Constants */

#define DatabaseIdx_salt_Login       0
#define DatabaseIdx_salt_Admin       10
#define DatabaseIdx_ipCount_SignUp   5
#define DatabaseIdx_ipCount_Login    1
#define DatabaseIdx_ipCount_Admin    12

#define REQUEST_MAX_SIZE 1024
#define SIGNUP_OK_TTL 3600
#define SIGNUP_FAILED_TTL 3610
#define SIGNLOCK_TTL 30
#define NEW_USER_TTL_30d 2592000
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
#define REDIS_TYPE "STRING:1, ARRAY:2, INTEGER:3, NIL:4, STATUS(OK):5, ERROR:6" 
// REDIS_REPLY_STRING REDIS_REPLY_INTEGER 

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
#include <threads.h>

/* Data structures */
struct session_request {
    int ver;
    char *username;
    char *passwd;
    char *signup_salt;
};

extern bool debug_mode ;
struct redis_config {
    char *path;
    bool is_unix;
    char *password;
    bool debug_mode;
};

extern char execBinaryMd5[] ;
/* Function declarations */
extern thread_local int postType_0signup_1login_2_admin ;
extern thread_local const char *postType_str ;
extern thread_local redisContext *ctx ;
extern thread_local int current_dbIdx ;
extern              const char *postReqAhead[] ;
extern                    int DbIdx_ipCountX ;
extern              const int DbIdx_salt[] ;


/* Debug and utility functions */
void print_debug(const char *fmt, ...);
#define DBprint_debug(fmt, ...)               do { if (debug_mode) { print_debug(fmt, ##__VA_ARGS__); } } while (0)
#define DXprint_debug(fmt, ...)      DBprint_debug("%s,%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
void print_debug_rn( const char *fmt, ...) ;
#define DBprint_debug_rn(fmt, ...)               do { if (debug_mode) { print_debug_rn(fmt, ##__VA_ARGS__); } } while (0)
#define DXprint_debug_rn(fmt, ...)      DBprint_debug_rn("%s,%d: " fmt, __func__, __LINE__, ##__VA_ARGS__)
void http_print_debug(const char *fmt, ...);
#define DBhttp_print_debug(fmt, ...)               do { if (debug_mode) { http_print_debug(fmt, ##__VA_ARGS__); } } while (0)
#define DXhttp_print_debug(fmt, ...)      DBhttp_print_debug("%s,%d: " fmt, __func__, __LINE__, ##__VA_ARGS__) 
void print_help(void);
bool parse_args(int argc, char *argv[], bool *debug, int *threads, struct redis_config *redis);

/* Redis functions */
int redis_check_ip(const char *ip, struct redis_config *conf, int dbIdx) ;
int redis_check_username(const char *username, struct redis_config *conf);
bool redis_store_user(const char *username, const char *hash, const char *salt, const char *ip, struct redis_config *conf);
int redis_increment( struct redis_config *conf, int dbIdx, int TTL, long *dstLong, const char *fmt, ...) ;
bool redis_connect_thread(struct redis_config *conf, int dbIdx);
int redis_save_key_to_redis_with_ttl(char databaseIdx, int TTL, const char *str1, const char *str2, const char *val, struct redis_config *conf) ;

/* Hash computation */
int compute_signup_hash2(const char *username, const char *passwd, char *hash, char *salt);

/* HTTP and server functions */
void set_redis_config(struct redis_config *conf);
void http_serve(void);
int http_should_keep_running(void);
int get_server_fd(void);
const char *extract_client_ip(const char *http_request);
bool parse_session_json(const char *body, struct session_request *req);
bool validate_username(const char *username);
bool validate_passwd(const char *passwd);
bool validate_signup_salt(const char *salt) ;
void send_response(int client_fd, int status, const char *status_text, char * cookieArr[], const char *fmt, ...) ;
char *http_get_client_ip(int client_fd, const char *buffer);

/* Thread functions */
void *thread_worker(void *arg);

void gen_a_new_md5sum_hex_32byte(char *output) ;

int get_executable_md5(unsigned char *md5_digest) ;
char *get_executable_md5_hex(void) ;
bool string_check_a2f_0to9( char * buf , int len ) ;
int cookie_extract(const char *buffer, size_t n, char *output_buf, size_t output_buf_size, const char *fmt, ...) ;
int redis_get_hget_string(struct redis_config *conf, int databaseIdx, int dstLen, char *dstBuf, const char *fmt, ... ) ;
int redis_get_int(struct redis_config *conf, int databaseIdx, long *dstInt, const char *fmt, ... ) ;
int redis_set_hset_key_value(struct redis_config *conf, int databaseIdx, int TTL, const char *fmt, ... ) ;
int check_post_type_path(char *buffer, size_t n) ;
int sess_handle_new_request(struct redis_config *conf, struct session_request *req , int client_fd ) ;
void send_response_sess2(int client_fd, char *sess, char *sesv ) ;
void send_response_sess3(int client_fd, char *sess, char *sesv, char *salt ) ;

#endif /* COMMON_H */
