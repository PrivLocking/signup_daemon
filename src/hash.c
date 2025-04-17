#include "common.h"

/*
const DEFAULT_CONFIG = {
    time: 2,            // Iterations
    mem: 131072,        // Memory (128 MB)
    hashLen: 32,        // Output hash length
    parallelism: 2,     // Parallelism factor
    type: 2             // Argon2id (0=Argon2d, 1=Argon2i, 2=Argon2id)
};
1.
client check if there is a signup_session
1.1, if exist, use it.
1.2, if no-exist, client submit a null json "{}" to server.
1.2.1. server received '{}', gen a 16byte(32hex ascii) signup_session, return 200 set set cookies(300s).
1.2.2. server storage signup_session, set TTL 300s.
2.
client use exist or new received signup_session, calc :
sha256_argon2_salt_username_password = sha256(argon2( signup_session + ":" + username + ":" + realpassword ))
then the client submit { username, sha256_argon2_salt_username_password }
3. server analyze cookies header and check for signup_session, try to match database.
3.1 if database not found such a signup_session, reject.
3.2 if database found such a signup_session, check if exist such a username,
3.3 if no double username, then save { username, signup_session, sha256_argon2_salt_username_password } into database as a new user record.
4. when login:
4.1: client submit {username} to server's '/login/'
4.1.1: server try to find username in database.
4.1.1.1: if not found, reject.
4.1.1.2: if found username, server gen and set a cookie: login_session(32 char hex ascii)/300s, return 200 with body { salt:signup_session }
4.2. the client calc :
login_hash = sha256(argon2(login_session + ":" + sha256(argon2( signup_session + ":" + username + ":" + realpassword ))))
then submit { username, login_hash }
4.3 the server check whether there is a login_session in temp database, if not exist, reject.
4.4 if exist, the server calc:
sha256_argon2_salt_username_password = query ( username )
sha256(argon2( login_session + ":" + sha256_argon2_salt_username_password  ))
and compare the result,
4.4.1 : if not equal, reject.
4.4.2 : if equal :
4.4.2.1 : generate a 16byte(32byte hexascii), strorage in datebase: username, login_auth=XXXXX
4.4.2.2 : and set cookie: Path:/ login_auth=XXXXX, 18000s
the above is my signup and login solution to keep user password safe. analyze it for me ? is there obversiual problem ?
*/
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
