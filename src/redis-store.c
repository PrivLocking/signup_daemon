#include "common.h"

extern bool redis_connect_thread(struct redis_config *conf);

bool redis_store_user(const char *username, const char *hash, const char *salt, const char *ip, struct redis_config *conf) {
    if (!redis_connect_thread(conf)) return false;

    redisContext *ctx = NULL;
    struct timeval timeout = { REDIS_TIMEOUT_SEC, 0 };
    ctx = redisConnectUnixWithTimeout(conf->path, timeout);
    if (!ctx || ctx->err) {
        print_debug("Redis connect failed on %s: %s", conf->path, ctx ? ctx->errstr : "null context");
        if (ctx) redisFree(ctx);
        return false;
    }

    redisReply *reply = redisCommand(ctx, "AUTH %s", conf->password);
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis AUTH failed: %s", reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
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
        redisFree(ctx);
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
        redisFree(ctx);
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
        redisFree(ctx);
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
        redisFree(ctx);
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
        redisFree(ctx);
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
        redisFree(ctx);
        return false;
    }
    freeReplyObject(reply);

    redisFree(ctx);
    return true;
}
