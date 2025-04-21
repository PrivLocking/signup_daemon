#include "common.h"

extern bool redis_connect_thread(struct redis_config *conf);

// rt = redis_get_string(redis_conf, 5, 32, dbSavedSignUpSalt, "signup_sess:%d", req.signup_salt ) ;
int redis_get_string(struct redis_config *conf, int databaseIdx, int dstLen, char *dstBuf, const char *fmt, ... ) {
    char srcBuf[1024] ;
    va_list args;
    va_start(args, fmt);
    vsnprintf(srcBuf,1024, fmt, args);
    va_end(args);


    if (!redis_connect_thread(conf)) return 125001;

    redisContext *ctx = NULL;
    struct timeval timeout = { REDIS_TIMEOUT_SEC, 0 };
    ctx = redisConnectUnixWithTimeout(conf->path, timeout);
    if (!ctx || ctx->err) {
        print_debug("Redis connect failed on %s: %s", conf->path, ctx ? ctx->errstr : "null context");
        if (ctx) redisFree(ctx);
        return 125003;
    }

    redisReply *reply = redisCommand(ctx, "AUTH %s", conf->password);
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis AUTH failed: %s", reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return 125005;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "SELECT %d", databaseIdx);
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis SELECT %d failed: %s", databaseIdx, reply ? reply->str : "null reply");
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return 125007;
    }
    freeReplyObject(reply);

    reply = redisCommand(ctx, "GET %s", srcBuf);
    if (conf->debug_mode) {
        print_debug("Sent: GET %s | Received: %s", srcBuf, reply && reply->str ? reply->str : "null");
    }
    //int count = 0;
    if (reply && reply->type == REDIS_REPLY_STRING) {
        //count = atoi(reply->str);
        strncpy( dstBuf, reply->str, dstLen ) ;
        int actual_len = strlen( dstBuf ) ;
        if ( dstLen != actual_len ) {
            print_debug("str copy [%s] failed. %d vs %d ", srcBuf, dstLen, actual_len);
            freeReplyObject(reply);
            redisFree(ctx);
            return 125011;
        }
    } else if (!reply || reply->type == REDIS_REPLY_ERROR) {
        print_debug("Redis GET %s failed", srcBuf);
        if (reply) freeReplyObject(reply);
        redisFree(ctx);
        return 125013;
    }
    freeReplyObject(reply);
    redisFree(ctx);
    return 0;
}

