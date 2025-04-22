#include "common.h"

__thread redisContext *ctx = NULL;
__thread int current_dbIdx = -1;

bool redis_connect_thread(struct redis_config *conf, int dbIdx) {
    redisReply *reply ;
    if (ctx) {
        if ( current_dbIdx == dbIdx ) return true ;

        reply = redisCommand(ctx, "SELECT %d", dbIdx);
        if (!reply || reply->type != REDIS_REPLY_STATUS) {
            DXprint_debug("Redis SELECT %d failed: %s", dbIdx, reply ? reply->str : "null reply");
            if (reply) freeReplyObject(reply);
            redisFree(ctx);
            ctx = NULL;
            current_dbIdx = -1 ;
            return false;
        }
        current_dbIdx = dbIdx ;
        return true;
    }

    current_dbIdx = -1 ;
    struct timeval timeout = { REDIS_TIMEOUT_SEC, 0 };
    ctx = redisConnectUnixWithTimeout(conf->path, timeout);
    if (!ctx || ctx->err) {
        DXprint_debug("Redis connect failed on %s: %s", conf->path, ctx ? ctx->errstr : "null context");
        if (ctx) redisFree(ctx);
        ctx = NULL;
        return false;
    }

    DXprint_debug("Connected to Redis on %s", conf->path);

    reply = redisCommand(ctx, "AUTH %s", conf->password);
    if (!reply || reply->type != REDIS_REPLY_STATUS) {
        DXprint_debug("Redis AUTH failed: %s", reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        ctx = NULL;
        return false;
    }
    DXprint_debug("Sent: AUTH <password> | Received: %s", reply->str);
    freeReplyObject(reply);

    reply = redisCommand(ctx, "SELECT %d", dbIdx);
    if (!reply || reply->type != REDIS_REPLY_STATUS) {
        DXprint_debug("Redis SELECT %d failed: %s", dbIdx, reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        ctx = NULL;
        return false;
    }
    DXprint_debug("Sent: SELECT %d | Received: %s", dbIdx, reply->str);

    freeReplyObject(reply);
    current_dbIdx = dbIdx ;
    return true;
}
