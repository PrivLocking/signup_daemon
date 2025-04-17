#include "common.h"

extern bool redis_connect_thread(struct redis_config *conf);

bool redis_increment_failed(const char *ip, struct redis_config *conf) {
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

    reply = redisCommand(ctx, "SELECT 5");
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT 5 failed: %s", reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return false;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "INCR signupFailed:%s:count", ip);
    if (conf->debug_mode) {
        print_debug("Sent: INCR signupFailed:%s:count | Received: %lld", ip, reply ? reply->integer : 0);
    }
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        print_debug("Redis INCR signupFailed:%s:count failed", ip);
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
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
        redisFree(ctx);
        return false;
    }
    freeReplyObject(reply);

    redisFree(ctx);
    return true;
}

