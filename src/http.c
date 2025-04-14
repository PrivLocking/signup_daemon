#include "common.h"

static struct redis_config *redis_conf = NULL;

void set_redis_config(struct redis_config *conf) {
    redis_conf = conf;
}

void http_print_debug(const char *fmt, ...) {
    if (!redis_conf || !redis_conf->debug_mode) return;
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[Thread %p] ", (void*)pthread_self());
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

static bool validate_username(const char *username) {
    if (!username || strlen(username) < USERNAME_MIN_LEN || strlen(username) > USERNAME_MAX_LEN) {
        http_print_debug("Username validation failed: length %zu (min %d, max %d)",
                         username ? strlen(username) : 0, USERNAME_MIN_LEN, USERNAME_MAX_LEN);
        return false;
    }
    if (!isalpha(username[0]) || !isalnum(username[strlen(username) - 1])) {
        http_print_debug("Username validation failed: must start with letter, end with letter/number");
        return false;
    }
    for (size_t i = 1; i < strlen(username) - 1; i++) {
        if (!isalnum(username[i]) && username[i] != '-' && username[i] != '_') {
            http_print_debug("Username validation failed: invalid char '%c' at pos %zu",
                             username[i], i);
            return false;
        }
    }
    return true;
}

static bool validate_passwd(const char *passwd) {
    if (!passwd || strlen(passwd) != PASSWD_LEN) {
        http_print_debug("Password validation failed: length %zu (expected %d)",
                         passwd ? strlen(passwd) : 0, PASSWD_LEN);
        return false;
    }
    for (size_t i = 0; i < PASSWD_LEN; i++) {
        if (!isdigit(passwd[i]) && !(passwd[i] >= 'a' && passwd[i] <= 'f')) {
            http_print_debug("Password validation failed: non-hex char at pos %zu", i);
            return false;
        }
    }
    bool all_zero = true;
    for (size_t i = 0; i < PASSWD_LEN; i++) {
        if (passwd[i] != '0') {
            all_zero = false;
            break;
        }
    }
    if (all_zero) {
        http_print_debug("Password validation failed: all zeros");
        return false;
    }
    return !all_zero;
}

static bool parse_json(const char *body, struct signup_request *req) {
    json_error_t error;
    json_t *root = json_loads(body, 0, &error);
    if (!root) {
        http_print_debug("JSON parse error: %s", error.text);
        return false;
    }

    json_t *username = json_object_get(root, "username");
    json_t *passwd = json_object_get(root, "passwd");
    if (!json_is_string(username) || !json_is_string(passwd)) {
        http_print_debug("JSON missing username or passwd");
        json_decref(root);
        return false;
    }

    if (json_object_size(root) > 2) {
        http_print_debug("JSON contains extra fields");
        json_decref(root);
        return false;
    }

    req->username = strdup(json_string_value(username));
    req->passwd = strdup(json_string_value(passwd));
    json_decref(root);

    for (size_t i = 0; req->username[i]; i++) {
        req->username[i] = tolower(req->username[i]);
    }

    http_print_debug("Parsed JSON: username=%s", req->username);
    return true;
}

static void send_response(int client_fd, int status, const char *status_text, const char *return_context) {
    char response[512];  // Increased buffer size for safety
    int content_length = 0;
    const char *body = "";

    if (return_context != NULL) {
        body = return_context;
        content_length = strlen(return_context);
    }

    snprintf(response, sizeof(response),
             "HTTP/1.1 %d %s\r\n"
             "Content-Length: %d\r\n"
             "Cache-Control: no-cache, no-store\r\n"
             "\r\n"
             "%s",
             status, status_text,
             content_length,
             body);

    write(client_fd, response, strlen(response));
}

void http_serve(void) {
    int server_fd = get_server_fd();
    if (server_fd == -1) {
        http_print_debug("Invalid server socket");
        return;
    }

    struct timeval timeout = { 1, 0 };
    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        http_print_debug("Setsockopt timeout failed: %s", strerror(errno));
        return;
    }

    while (http_should_keep_running()) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            if (errno == EINTR) continue;
            http_print_debug("Accept failed: %s", strerror(errno));
            sleep(1);
            continue;
        }

        http_print_debug("Accepted client connection");

        char buffer[REQUEST_MAX_SIZE + 1];
        ssize_t n = read(client_fd, buffer, REQUEST_MAX_SIZE);
        if (n <= 0) {
            http_print_debug("Read failed or connection closed");
            close(client_fd);
            continue;
        }
        buffer[n] = '\0';
        http_print_debug("buffer is :[%s]", buffer);

        if (strncmp(buffer, "POST /signup", 12) != 0) {
            http_print_debug("Invalid request, expected POST /signup, got: %.12s", buffer);
            send_response(client_fd, 422, "Unprocessable Entity", "10");
            close(client_fd);
            continue;
        }

        char *body = strstr(buffer, "\r\n\r\n");
        if (!body) {
            http_print_debug("No request body found");
            send_response(client_fd, 422, "Unprocessable Entity", "11");
            close(client_fd);
            continue;
        }
        body += 4;

        struct signup_request req = { NULL, NULL };
        if (!parse_json(body, &req)) {
            send_response(client_fd, 422, "Unprocessable Entity", "12");
            if (req.username) free(req.username);
            if (req.passwd) free(req.passwd);
            close(client_fd);
            continue;
        }

        if (!validate_username(req.username) || !validate_passwd(req.passwd)) {
            send_response(client_fd, 422, "Unprocessable Entity", "13");
            free(req.username);
            free(req.passwd);
            close(client_fd);
            continue;
        }

        char ip[] = "127.0.0.1";
        if (!redis_check_ip(ip, redis_conf)) {
            http_print_debug("IP check failed for %s", ip);
            send_response(client_fd, 422, "Unprocessable Entity", "14");
            free(req.username);
            free(req.passwd);
            close(client_fd);
            continue;
        }

        if (!redis_check_username(req.username, redis_conf)) {
            http_print_debug("Username check failed for %s", req.username);
            redis_increment_failed(ip, redis_conf);
            send_response(client_fd, 422, "Unprocessable Entity", "15");
            free(req.username);
            free(req.passwd);
            close(client_fd);
            continue;
        }

        char hash[HASH_LEN + 1], salt[SALT_LEN + 1];
        if (!compute_hash(req.username, req.passwd, hash, salt)) {
            http_print_debug("Hash computation failed");
            redis_increment_failed(ip, redis_conf);
            send_response(client_fd, 422, "Unprocessable Entity", "16");
            free(req.username);
            free(req.passwd);
            close(client_fd);
            continue;
        }

        if (!redis_store_user(req.username, hash, salt, ip, redis_conf)) {
            http_print_debug("Failed to store user in Redis");
            redis_increment_failed(ip, redis_conf);
            send_response(client_fd, 503, "Service Unavailable", NULL);
            free(req.username);
            free(req.passwd);
            close(client_fd);
            continue;
        }

        http_print_debug("User signup successful");
        send_response(client_fd, 200, "OK", NULL );
        free(req.username);
        free(req.passwd);
        close(client_fd);
    }
}
