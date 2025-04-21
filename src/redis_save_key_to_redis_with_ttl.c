#include "common.h"

bool redis_save_key_to_redis_with_ttl(char databaseIdx, int TTL, const char *str1, const char *str2, const char *val, struct redis_config *conf) {
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

    //reply = redisCommand(ctx, "SELECT 5");
    reply = redisCommand(ctx, "SELECT %d", databaseIdx );
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT 5 failed: %s", reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return false;
    }
    freeReplyObject(reply);

    // Store session with 300 seconds TTL
    reply = redisCommand(ctx, "SET %s:%s %s EX %d", str1, str2, val, TTL);
    if (conf->debug_mode) {
        print_debug("Sent: SET %s:%s %s EX %d | Received: %s", str1, str2, val, TTL, reply ? reply->str : "null");
    }
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SET %s:%s %s EX %d failed", str1, str2, val, TTL);
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return false;
    }
    freeReplyObject(reply);

    redisFree(ctx);
    return true;
}
