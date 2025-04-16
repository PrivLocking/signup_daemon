#include "common.h"

extern bool redis_connect_thread(struct redis_config *conf);

bool redis_check_username(const char *username, struct redis_config *conf) {
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

    reply = redisCommand(ctx, "EXISTS user:%s", username);
    if (conf->debug_mode) {
        print_debug("Sent: EXISTS user:%s | Received: %lld", username, reply ? reply->integer : 0);
    }
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        print_debug("Redis EXISTS user:%s failed", username);
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return false;
    }
    bool exists = reply->integer == 1;
    freeReplyObject(reply);
    if (exists) {
        redisFree(ctx);
        return false;
    }

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

    reply = redisCommand(ctx, "SET signlock:%s 1 NX EX %d", username, SIGNLOCK_TTL);
    if (conf->debug_mode) {
        print_debug("Sent: SET signlock:%s 1 NX EX %d | Received: %s", username, SIGNLOCK_TTL, reply && reply->str ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_NIL) {
        print_debug("Redis SET signlock:%s failed", username);
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

    reply = redisCommand(ctx, "EXISTS user:%s", username);
    if (conf->debug_mode) {
        print_debug("Sent: EXISTS user:%s | Received: %lld", username, reply ? reply->integer : 0);
    }
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        print_debug("Redis EXISTS user:%s failed", username);
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return false;
    }
    exists = reply->integer == 1;
    freeReplyObject(reply);

    redisFree(ctx);
    return !exists;
}
