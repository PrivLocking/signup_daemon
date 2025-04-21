#include "common.h"

bool redis_save_signup_sess_with_TTL300(const char *signup_sess, struct redis_config *conf) {
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

    // Store session with 300 seconds TTL
    reply = redisCommand(ctx, "SET signup_sess:%s 1 EX 300", signup_sess);
    if (conf->debug_mode) {
        print_debug("Sent: SET signup_sess:%s 1 EX 300 | Received: %s", signup_sess, reply ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SET signup_sess:%s failed", signup_sess);
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return false;
    }
    freeReplyObject(reply);

    redisFree(ctx);
    return true;
}

