#include "common.h"

#include <openssl/rand.h>
#include <argon2.h>

bool compute_hash(const char *username, const char *passwd, char *hash, char *salt) {
    uint8_t salt_bytes[SALT_LEN / 2];
    if (RAND_bytes(salt_bytes, sizeof(salt_bytes)) != 1) {
        return false;
    }

    for (size_t i = 0; i < sizeof(salt_bytes); i++) {
        snprintf(salt + i * 2, 3, "%02x", salt_bytes[i]);
    }
    salt[SALT_LEN] = '\0';

    uint8_t hash_bytes[HASH_LEN / 2];
    if (argon2id_hash_raw(2, 1 << 16, 1, passwd, strlen(passwd),
                          salt_bytes, sizeof(salt_bytes),
                          hash_bytes, sizeof(hash_bytes)) != ARGON2_OK) {
        return false;
    }

    for (size_t i = 0; i < sizeof(hash_bytes); i++) {
        snprintf(hash + i * 2, 3, "%02x", hash_bytes[i]);
    }
    hash[HASH_LEN] = '\0';

    return true;
}
