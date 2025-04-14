#include "common.h"

static __thread redisContext *ctx = NULL;

static bool redis_connect_thread(struct redis_config *conf) {
    if (ctx) {
        redisFree(ctx);
        ctx = NULL;
    }
    struct timeval timeout = { REDIS_TIMEOUT_SEC, 0 };
    ctx = redisConnectUnixWithTimeout(conf->path, timeout);
    if (!ctx || ctx->err) {
        print_debug("Redis connect failed on %s: %s", conf->path, ctx ? ctx->errstr : "null context");
        if (ctx) redisFree(ctx);
        ctx = NULL;
        return false;
    }

    if (conf->debug_mode) {
        print_debug("Connected to Redis on %s", conf->path);
    }

    redisReply *reply = redisCommand(ctx, "AUTH %s", conf->password);
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis AUTH failed: %s", reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        ctx = NULL;
        return false;
    }
    if (conf->debug_mode) {
        print_debug("Sent: AUTH <password> | Received: %s", reply->str);
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "SELECT 5");
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT 5 failed: %s", reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        ctx = NULL;
        return false;
    }
    if (conf->debug_mode) {
        print_debug("Sent: SELECT 5 | Received: %s", reply->str);
    }
    freeReplyObject(reply);

    return true;
}

bool redis_check_ip(const char *ip, struct redis_config *conf) {
    if (!ctx && !redis_connect_thread(conf)) return false;

    redisReply *reply = redisCommand(ctx, "SELECT 5");
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT 5 failed: %s", reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        return false;
    }
    if (conf->debug_mode) {
        print_debug("Sent: SELECT 5 | Received: %s", reply->str);
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "EXISTS signupOK:%s", ip);
    if (conf->debug_mode) {
        print_debug("Sent: EXISTS signupOK:%s | Received: %lld", ip, reply ? reply->integer : 0);
    }
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        print_debug("Redis EXISTS signupOK:%s failed", ip);
        if (reply) freeReplyObject(reply);
        return false;
    }
    bool exists = reply->integer == 1;
    freeReplyObject(reply);
    if (exists) {
        reply = redisCommand(ctx, "INCR signupFailed:%s:count", ip);
        if (conf->debug_mode) {
            print_debug("Sent: INCR signupFailed:%s:count | Received: %lld", ip, reply ? reply->integer : 0);
        }
        if (reply && reply->type == REDIS_REPLY_INTEGER) {
            reply = redisCommand(ctx, "EXPIRE signupFailed:%s:count %d", ip, SIGNUP_FAILED_TTL);
            if (conf->debug_mode) {
                print_debug("Sent: EXPIRE signupFailed:%s:count %d | Received: %lld", ip, SIGNUP_FAILED_TTL, reply ? reply->integer : 0);
            }
            if (!reply || reply->type != REDIS_REPLY_INTEGER) {
                print_debug("Redis EXPIRE signupFailed:%s:count failed", ip);
            }
            if (reply) freeReplyObject(reply);
        } else {
            print_debug("Redis INCR signupFailed:%s:count failed", ip);
            if (reply) freeReplyObject(reply);
        }
        return false;
    }

    reply = redisCommand(ctx, "GET signupFailed:%s:count", ip);
    if (conf->debug_mode) {
        print_debug("Sent: GET signupFailed:%s:count | Received: %s", ip, reply && reply->str ? reply->str : "null");
    }
    int count = 0;
    if (reply && reply->type == REDIS_REPLY_STRING) {
        count = atoi(reply->str);
    } else if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis GET signupFailed:%s:count failed", ip);
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    if (count >= 5) {
        reply = redisCommand(ctx, "INCR signupFailed:%s:count", ip);
        if (conf->debug_mode) {
            print_debug("Sent: INCR signupFailed:%s:count | Received: %lld", ip, reply ? reply->integer : 0);
        }
        if (!reply || reply->type != REDIS_REPLY_INTEGER) {
            print_debug("Redis INCR signupFailed:%s:count failed", ip);
            if (reply) freeReplyObject(reply);
            return false;
        }
        freeReplyObject(reply);

        reply = redisCommand(ctx, "EXPIRE signupFailed:%s:count %d", ip, SIGNUP_FAILED_TTL);
        if (conf->debug_mode) {
            print_debug("Sent: EXPIRE signupFailed:%s:count %d | Received: %lld", ip, SIGNUP_FAILED_TTL, reply ? reply->integer : 0);
        }
        if (!reply || reply->type != REDIS_REPLY_INTEGER) {
            print_debug("Redis EXPIRE signupFailed:%s:count failed", ip);
            if (reply) freeReplyObject(reply);
            return false;
        }
        freeReplyObject(reply);

        return false;
    }

    return true;
}

bool redis_check_username(const char *username, struct redis_config *conf) {
    if (!ctx && !redis_connect_thread(conf)) return false;

    redisReply *reply = redisCommand(ctx, "SELECT 0");
    if (conf->debug_mode) {
        print_debug("Sent: SELECT 0 | Received: %s", reply ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT 0 failed");
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "EXISTS user:%s", username);
    if (conf->debug_mode) {
        print_debug("Sent: EXISTS user:%s | Received: %lld", username, reply ? reply->integer : 0);
    }
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        print_debug("Redis EXISTS user:%s failed", username);
        if (reply) freeReplyObject(reply);
        return false;
    }
    bool exists = reply->integer == 1;
    freeReplyObject(reply);
    if (exists) return false;

    reply = redisCommand(ctx, "SELECT 5");
    if (conf->debug_mode) {
        print_debug("Sent: SELECT 5 | Received: %s", reply ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT 5 failed");
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "SET signlock:%s 1 NX EX %d", username, SIGNLOCK_TTL);
    if (conf->debug_mode) {
        print_debug("Sent: SET signlock:%s 1 NX EX %d | Received: %s", username, SIGNLOCK_TTL, reply && reply->str ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_NIL) {
        print_debug("Redis SET signlock:%s failed", username);
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "SELECT 0");
    if (conf->debug_mode) {
        print_debug("Sent: SELECT 0 | Received: %s", reply ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT 0 failed");
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "EXISTS user:%s", username);
    if (conf->debug_mode) {
        print_debug("Sent: EXISTS user:%s | Received: %lld", username, reply ? reply->integer : 0);
    }
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        print_debug("Redis EXISTS user:%s failed", username);
        if (reply) freeReplyObject(reply);
        return false;
    }
    exists = reply->integer == 1;
    freeReplyObject(reply);

    reply = redisCommand(ctx, "SELECT 5");
    if (conf->debug_mode) {
        print_debug("Sent: SELECT 5 | Received: %s", reply ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT 5 failed");
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    return !exists;
}

bool redis_store_user(const char *username, const char *hash, const char *salt, const char *ip, struct redis_config *conf) {
    if (!ctx && !redis_connect_thread(conf)) return false;

    redisReply *reply = redisCommand(ctx, "SELECT 0");
    if (conf->debug_mode) {
        print_debug("Sent: SELECT 0 | Received: %s", reply ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT 0 failed");
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "HMSET user:%s hash %s salt %s level 0 status active",
                        username, hash, salt);
    if (conf->debug_mode) {
        print_debug("Sent: HMSET user:%s hash %s salt %s level 0 status active | Received: %s",
                    username, hash, salt, reply ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis HMSET user:%s failed", username);
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "EXPIRE user:%s %d", username, USER_TTL);
    if (conf->debug_mode) {
        print_debug("Sent: EXPIRE user:%s %d | Received: %lld", username, USER_TTL, reply ? reply->integer : 0);
    }
    if (!reply || reply->type != REDIS_REPLY_INTEGER || reply->integer != 1) {
        print_debug("Redis EXPIRE user:%s failed", username);
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "SELECT 5");
    if (conf->debug_mode) {
        print_debug("Sent: SELECT 5 | Received: %s", reply ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT 5 failed");
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "SET signupOK:%s 1 EX %d", ip, SIGNUP_OK_TTL);
    if (conf->debug_mode) {
        print_debug("Sent: SET signupOK:%s 1 EX %d | Received: %s", ip, SIGNUP_OK_TTL, reply ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SET signupOK:%s failed", ip);
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "SET signupFailed:%s:count 0 EX %d", ip, SIGNUP_FAILED_TTL);
    if (conf->debug_mode) {
        print_debug("Sent: SET signupFailed:%s:count 0 EX %d | Received: %s", ip, SIGNUP_FAILED_TTL, reply ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SET signupFailed:%s:count failed", ip);
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    return true;
}

bool redis_increment_failed(const char *ip, struct redis_config *conf) {
    if (!ctx && !redis_connect_thread(conf)) return false;

    redisReply *reply = redisCommand(ctx, "SELECT 5");
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT 5 failed: %s", reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        return false;
    }
    if (conf->debug_mode) {
        print_debug("Sent: SELECT 5 | Received: %s", reply->str);
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "INCR signupFailed:%s:count", ip);
    if (conf->debug_mode) {
        print_debug("Sent: INCR signupFailed:%s:count | Received: %lld", ip, reply ? reply->integer : 0);
    }
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        print_debug("Redis INCR signupFailed:%s:count failed", ip);
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "EXPIRE signupFailed:%s:count %d", ip, SIGNUP_FAILED_TTL);
    if (conf->debug_mode) {
        print_debug("Sent: EXPIRE signupFailed:%s:count %d | Received: %lld", ip, SIGNUP_FAILED_TTL, reply ? reply->integer : 0);
    }
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        print_debug("Redis EXPIRE signupFailed:%s:count failed", ip);
        if (reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);

    return true;
}
