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
        DBhttp_print_debug("Username validation failed: length %zu (min %d, max %d)",
                         username ? strlen(username) : 0, USERNAME_MIN_LEN, USERNAME_MAX_LEN);
        return false;
    }
    if (!isalpha(username[0]) || !isalnum(username[strlen(username) - 1])) {
        DBhttp_print_debug("Username validation failed: must start with letter, end with letter/number");
        return false;
    }
    for (size_t i = 1; i < strlen(username) - 1; i++) {
        if (!isalnum(username[i]) && username[i] != '-' && username[i] != '_') {
            DBhttp_print_debug("Username validation failed: invalid char '%c' at pos %zu",
                             username[i], i);
            return false;
        }
    }
    return true;
}

bool validate_passwd(const char *passwd) {
    if (!passwd || strlen(passwd) != PASSWD_LEN) {
        DBhttp_print_debug("Password validation failed: length %zu (expected %d)",
                         passwd ? strlen(passwd) : 0, PASSWD_LEN);
        return false;
    }
    for (size_t i = 0; i < PASSWD_LEN; i++) {
        if (!isdigit(passwd[i]) && !(passwd[i] >= 'a' && passwd[i] <= 'f')) {
            DBhttp_print_debug("Password validation failed: non-hex char at pos %zu", i);
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
        DBhttp_print_debug("Password validation failed: all zeros");
        return false;
    }
    return true;
}

bool validate_signup_salt(const char *salt) {
    if (!salt || strlen(salt) != SALT_LEN) {
        DBhttp_print_debug("Salt validation failed: length %zu (expected %d)",
                         salt ? strlen(salt) : 0, SALT_LEN);
        return false;
    }
    for (size_t i = 0; i < SALT_LEN; i++) {
        if (!isdigit(salt[i]) && !(salt[i] >= 'a' && salt[i] <= 'f')) {
            DBhttp_print_debug("Salt validation failed: non-hex char at pos %zu", i);
            return false;
        }
    }
    bool all_zero = true;
    for (size_t i = 0; i < SALT_LEN; i++) {
        if (salt[i] != '0') {
            all_zero = false;
            break;
        }
    }
    if (all_zero) {
        DBhttp_print_debug("Salt validation failed: all zeros");
        return false;
    }
    return true;
}

