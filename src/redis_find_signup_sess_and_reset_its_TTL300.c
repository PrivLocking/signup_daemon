#include "common.h"

bool redis_find_signup_sess_and_reset_its_TTL300(const char *signup_sess, struct redis_config *conf) {
    if (!redis_connect_thread(conf, 5)) return false;

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

    // Check if the session exists
    reply = redisCommand(ctx, "EXISTS signup_sess:%s", signup_sess);
    if (conf->debug_mode) {
        print_debug("Sent: EXISTS signup_sess:%s | Received: %lld", signup_sess, reply ? reply->integer : 0);
    }
    if (!reply || reply->type != REDIS_REPLY_INTEGER) {
        print_debug("Redis EXISTS signup_sess:%s failed", signup_sess);
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return false;
    }
    bool exists = reply->integer == 1;
    freeReplyObject(reply);

    if (exists) {
        // Reset TTL to 300 seconds
        reply = redisCommand(ctx, "EXPIRE signup_sess:%s 300", signup_sess);
        if (conf->debug_mode) {
            print_debug("Sent: EXPIRE signup_sess:%s 300 | Received: %lld", signup_sess, reply ? reply->integer : 0);
        }
        if (!reply || reply->type != REDIS_REPLY_INTEGER || reply->integer != 1) {
            print_debug("Redis EXPIRE signup_sess:%s failed", signup_sess);
            if (reply) freeReplyObject(reply);
            redisFree(ctx);
            return false;
        }
        freeReplyObject(reply);
    }

    redisFree(ctx);
    return exists;
}
