#include "common.h"

static __thread redisContext *ctx = NULL;

bool redis_connect_thread(struct redis_config *conf) {
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
