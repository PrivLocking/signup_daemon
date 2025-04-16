#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include "common.h"

void http_print_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "[Thread %p] ", (void*)pthread_self());
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

bool validate_username(const char *username) {
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

bool validate_passwd(const char *passwd) {
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

char *get_client_ip(int client_fd, const char *buffer) {
    // First, try to extract from HTTP headers
    if (buffer) {
        // Try to get X-Real-IP
        const char *x_real_ip = strcasestr(buffer, "\nX-Real-IP: ");
        if (x_real_ip) {
            x_real_ip += 12; // Skip header name
            char *ip = malloc(16); // IPv4 max len
            if (!ip) return strdup("127.0.0.2");
            
            int i = 0;
            while (*x_real_ip && *x_real_ip != '\r' && *x_real_ip != '\n' && i < 15) {
                ip[i++] = *x_real_ip++;
            }
            ip[i] = '\0';
            http_print_debug("Found X-Real-IP: %s", ip);
            return ip;
        }
        
        // Try to get X-Forwarded-For
        const char *x_forwarded_for = strcasestr(buffer, "\nX-Forwarded-For: ");
        if (x_forwarded_for) {
            x_forwarded_for += 17; // Skip header name
            char *ip = malloc(16); // IPv4 max len
            if (!ip) return strdup("127.0.0.2");
            
            int i = 0;
            while (*x_forwarded_for && *x_forwarded_for != '\r' && *x_forwarded_for != '\n' && *x_forwarded_for != ',' && i < 15) {
                ip[i++] = *x_forwarded_for++;
            }
            ip[i] = '\0';
            http_print_debug("Found X-Forwarded-For: %s", ip);
            return ip;
        }
    }
    
    // If we couldn't extract from headers, try to get from socket
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);
    if (getpeername(client_fd, (struct sockaddr*)&addr, &addrlen) == 0) {
        if (addr.ss_family == AF_INET) {
            char *ip = malloc(16); // IPv4 max len
            if (!ip) return strdup("127.0.0.2");
            
            struct sockaddr_in *s = (struct sockaddr_in*)&addr;
            inet_ntop(AF_INET, &s->sin_addr, ip, 16);
            http_print_debug("Extracted socket IP: %s", ip);
            return ip;
        }
    }
    
    // Default IP
    http_print_debug("Using default IP: 127.0.0.2");
    return strdup("127.0.0.2");
}

void send_response(int client_fd, int status, const char *status_text, const char *return_context) {
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
